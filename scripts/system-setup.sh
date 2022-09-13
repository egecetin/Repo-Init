#!/bin/bash
set -e

# Color Variables
ANSI_FG_BLACK="\x1b[30m"
ANSI_FG_RED="\x1b[31m"
ANSI_FG_GREEN="\x1b[32m"
ANSI_FG_YELLOW="\x1b[33m"
ANSI_FG_BLUE="\x1b[34m"
ANSI_FG_MAGENTA="\x1b[35m"
ANSI_FG_CYAN="\x1b[36m"
ANSI_RESET_ALL="\x1b[0m"

# Versions
VER_PROMETHEUS=2.37.0
VER_NODE_EXPORTER=1.3.1

# Install
echo -e "${ANSI_FG_YELLOW}Installing packages ...${ANSI_RESET_ALL}"
yum install epel-release -y
yum install btop htop cockpit cockpit-pcp chrony mlocate lm_sensors smartmontools -y
yum install OpenIPMI ipmitool -y

echo -e "${ANSI_FG_YELLOW}Installing cockpit-navigator ...${ANSI_RESET_ALL}"
curl -sSL https://repo.45drives.com/setup | sudo bash
yum install cockpit-navigator -y

echo -e "${ANSI_FG_YELLOW}Installing neofetch ...${ANSI_RESET_ALL}"
wget https://github.com/dylanaraps/neofetch/archive/refs/tags/7.1.0.tar.gz
tar -xzvf 7.1.0.tar.gz neofetch-7.1.0/
make install -C neofetch-7.1.0
rm -rf neofetch-7.1.0
mkdir -p /root/.config/neofetch
\cp scripts/data/neofetch_config.conf /root/.config/neofetch/config.conf
echo "neofetch" >> /etc/profile.d/neofetch-init.sh

# echo -e "${ANSI_FG_YELLOW}Installing IDRAC tools ...${ANSI_RESET_ALL}"
# curl -O https://linux.dell.com/repo/hardware/dsu/bootstrap.cgi
# bash bootstrap.cgi
# rm -f bootstrap.cgi
# yum install dell-system-update -y
# yum install srvadmin-* -y
# cat << EOF >> /etc/ld.so.conf.d/idrac.conf
# /opt/dell/srvadmin/sbin/
# EOF
# ldconfig

echo -e "${ANSI_FG_YELLOW}Installing Intel oneAPI ...${ANSI_RESET_ALL}"
tee > /tmp/oneAPI.repo << EOF
[oneAPI]
name=Intel® oneAPI repository
baseurl=https://yum.repos.intel.com/oneapi
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://yum.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB
EOF
mv -f /tmp/oneAPI.repo /etc/yum.repos.d
yum install intel-oneapi-runtime-libs -y

echo -e "${ANSI_FG_YELLOW}Installing Prometheus v${VER_PROMETHEUS}...${ANSI_RESET_ALL}"
adduser -M -r -s /sbin/nologin prometheus
mkdir /etc/prometheus
mkdir /var/lib/prometheus

wget https://github.com/prometheus/prometheus/releases/download/v$VER_PROMETHEUS/prometheus-$VER_PROMETHEUS.linux-amd64.tar.gz -P /tmp
tar -xzf /tmp/prometheus-$VER_PROMETHEUS.linux-amd64.tar.gz --directory=/tmp
\cp /tmp/prometheus-$VER_PROMETHEUS.linux-amd64/{prometheus,promtool} /usr/local/bin/
\cp -r /tmp/prometheus-$VER_PROMETHEUS.linux-amd64/{consoles,console_libraries} /etc/prometheus/
\cp scripts/data/prometheus-config.yml /etc/prometheus/prometheus.yml

chown -R prometheus:prometheus /etc/prometheus
chown -R prometheus:prometheus /var/lib/prometheus
chown prometheus.prometheus /usr/local/bin/{prometheus,promtool}

tee > /etc/systemd/system/prometheus.service << EOF
[Unit]
Description=Prometheus
Wants=network-online.target
After=network-online.target

[Service]
User=prometheus
Group=prometheus
Type=simple
ExecStart=/usr/local/bin/prometheus \
    --config.file /etc/prometheus/prometheus.yml \
    --storage.tsdb.path /var/lib/prometheus/ \
    --web.console.templates=/etc/prometheus/consoles \
    --web.console.libraries=/etc/prometheus/console_libraries \
    --web.listen-address=:9080

[Install]
WantedBy=multi-user.target

EOF

echo -e "${ANSI_FG_YELLOW}Installing Prometheus Node Exporter v${VER_NODE_EXPORTER}...${ANSI_RESET_ALL}"
adduser -M -r -s /sbin/nologin node_exporter

wget https://github.com/prometheus/node_exporter/releases/download/v$VER_NODE_EXPORTER/node_exporter-$VER_NODE_EXPORTER.linux-amd64.tar.gz -P /tmp
tar -xf /tmp/node_exporter-$VER_NODE_EXPORTER.linux-amd64.tar.gz --directory=/tmp
mv /tmp/node_exporter-$VER_NODE_EXPORTER.linux-amd64/node_exporter /usr/local/bin/
\cp scripts/data/prometheus-firewalld.xml /etc/firewalld/services/prometheus.xml

chown node_exporter:node_exporter /usr/local/bin/node_exporter
restorecon -rv /usr/local/bin/node_exporter

tee > /etc/systemd/system/node_exporter.service << EOF
[Unit]
Description=Node Exporter
After=network.target

[Service]
User=node_exporter
Group=node_exporter
Type=simple
ExecStart=/usr/local/bin/node_exporter

[Install]
WantedBy=multi-user.target

EOF

# echo -e "${ANSI_FG_YELLOW}Installing Hyper-V tools"
# yum install WALinuxAgent cloud-init cloud-utils-growpart gdisk hyperv-daemons -y

echo -e "${ANSI_FG_YELLOW}Detecting sensors ...${ANSI_RESET_ALL}"
sensors-detect --auto > /dev/null

echo -e "${ANSI_FG_YELLOW}Enabling services ...${ANSI_RESET_ALL}"
systemctl daemon-reload

/sbin/chkconfig ipmi on
systemctl start ipmi
systemctl enable --now chronyd
systemctl enable --now pmlogger
# systemctl enable --now waagent
# systemctl enable --now cloud-init
systemctl enable --now prometheus
systemctl enable --now node_exporter
systemctl enable --now cockpit.socket

systemctl stop dnf-makecache.timer
systemctl disable dnf-makecache.timer

echo -e "${ANSI_FG_YELLOW}Disabling non-secure ciphers ...${ANSI_RESET_ALL}"
echo "" >> /etc/ssh/sshd_config
echo "# Custom ciphers" >> /etc/ssh/sshd_config
echo "Ciphers aes128-ctr,aes192-ctr,aes256-ctr" >> /etc/ssh/sshd_config
echo "MACs hmac-sha1,umac-64@openssh.com" >> /etc/ssh/sshd_config
echo -e "${ANSI_FG_YELLOW}Restarting sshd service ...${ANSI_RESET_ALL}"
systemctl restart sshd.service

echo -e "${ANSI_FG_YELLOW}Branding ...${ANSI_RESET_ALL}"

mv -f /usr/share/cockpit/branding/rhel/branding.css /usr/share/cockpit/branding/rhel/branding.css.old
mv -f /usr/share/cockpit/branding/rhel/logo.png /usr/share/cockpit/branding/rhel/logo.png.old
mv -f /usr/share/cockpit/branding/rhel/apple-touch-icon.png /usr/share/cockpit/branding/rhel/apple-touch-icon.png.old
mv -f /usr/share/cockpit/branding/rhel/favicon.ico /usr/share/cockpit/branding/rhel/favicon.ico.old

\cp scripts/data/branding/branding.css /usr/share/cockpit/branding/rhel/branding.css
\cp scripts/data/branding/logo.png /usr/share/cockpit/branding/rhel/logo.png
\cp scripts/data/branding/apple-touch-icon.png /usr/share/cockpit/branding/rhel/apple-touch-icon.png
\cp scripts/data/branding/favicon.ico /usr/share/cockpit/branding/rhel/favicon.ico

semanage fcontext -a -t usr_t /usr/share/cockpit/branding/rhel/logo.png || true
semanage fcontext -a -t usr_t /usr/share/cockpit/branding/rhel/apple-touch-icon.png || true
semanage fcontext -a -t usr_t /usr/share/cockpit/branding/rhel/favicon.ico || true
restorecon -v /usr/share/cockpit/branding/rhel/logo.png || true
restorecon -v /usr/share/cockpit/branding/rhel/apple-touch-icon.png || true
restorecon -v /usr/share/cockpit/branding/rhel/favicon.ico || true
restorecon -v /usr/share/cockpit/branding/centos/branding.css || true

echo -e "${ANSI_FG_YELLOW}Configuring firewall ...${ANSI_RESET_ALL}"
firewall-cmd --reload
firewall-cmd --permanent --zone=public --add-service=ssh
firewall-cmd --permanent --zone=public --add-service=cockpit
firewall-cmd --permanent --zone=trusted --add-service=prometheus
firewall-cmd --reload

echo -e "${ANSI_FG_YELLOW}Displaying information ...${ANSI_RESET_ALL}"
echo ""
echo -e "${ANSI_FG_GREEN}Host${ANSI_RESET_ALL}"
hostnamectl
echo ""
echo ""
sleep 3
echo -e "${ANSI_FG_GREEN}NTP${ANSI_RESET_ALL}"
chronyc sources -v
echo ""
echo ""
sleep 3
echo -e "${ANSI_FG_GREEN}Network${ANSI_RESET_ALL}"
ip addr
echo ""
echo ""
sleep 3
echo -e "${ANSI_FG_GREEN}SELinux${ANSI_RESET_ALL}"
sestatus
echo ""
echo ""
sleep 3
echo -e "${ANSI_FG_GREEN}Firewall${ANSI_RESET_ALL}"
firewall-cmd --list-all
sleep 1
echo "${ANSI_FG_GREEN}Done!${ANSI_RESET_ALL}"
