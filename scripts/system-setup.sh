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
VER_LOKI=2.7.3
VER_PROMTAIL=2.7.3

# Install
echo -e "${ANSI_FG_YELLOW}Installing packages ...${ANSI_RESET_ALL}"
dnf install epel-release -y
dnf install wget htop cockpit chrony mlocate policycoreutils-python-utils setroubleshoot smartmontools -y

echo -e "${ANSI_FG_YELLOW}Installing cockpit-navigator ...${ANSI_RESET_ALL}"
curl -sSL https://repo.45drives.com/setup | sudo bash
dnf install cockpit-navigator -y

echo -e "${ANSI_FG_YELLOW}Installing neofetch ...${ANSI_RESET_ALL}"
pip install hyfetch
mkdir -p /root/.config/neofetch
\cp scripts/data/neofetch_config.conf /root/.config/neofetch/config.conf
echo "neowofetch" >> /etc/profile.d/neofetch.sh

# echo -e "${ANSI_FG_YELLOW}Installing IDRAC tools ...${ANSI_RESET_ALL}"
# curl -O https://linux.dell.com/repo/hardware/dsu/bootstrap.cgi
# bash bootstrap.cgi
# rm -f bootstrap.cgi
# dnf install dell-system-update -y
# dnf install srvadmin-* -y
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
dnf install intel-oneapi-runtime-libs -y

echo -e "${ANSI_FG_YELLOW}Installing Grafana... ${ANSI_RESET_ALL}"
tee > /etc/yum.repos.d/grafana.repo << EOF
[grafana]
name=grafana
baseurl=https://rpm.grafana.com
repo_gpgcheck=1
enabled=1
gpgcheck=1
gpgkey=https://rpm.grafana.com/gpg.key
sslverify=1
sslcacert=/etc/pki/tls/certs/ca-bundle.crt
exclude=*beta*
EOF
dnf install grafana -y
\cp scripts/data/grafana-firewalld.xml /etc/firewalld/services/grafana.xml
\cp scripts/data/grafana.ini /etc/grafana/grafana.ini

echo -e "${ANSI_FG_YELLOW}Installing Loki v${VER_LOKI}...${ANSI_RESET_ALL}"
wget https://github.com/grafana/loki/releases/download/v$VER_LOKI/loki-$VER_LOKI.x86_64.rpm
rpm -i loki-$VER_LOKI.x86_64.rpm
\cp scripts/data/loki-firewalld.xml /etc/firewalld/services/loki.xml

echo -e "${ANSI_FG_YELLOW}Installing Promtail v${VER_PROMTAIL}...${ANSI_RESET_ALL}"
wget https://github.com/grafana/loki/releases/download/v$VER_PROMTAIL/promtail-$VER_PROMTAIL.x86_64.rpm
rpm -i promtail-$VER_PROMTAIL.x86_64.rpm
systemctl stop promtail
usermod -a -G systemd-journal promtail
setfacl -R -m u:promtail:rX /var/log
\cp scripts/data/promtail-firewalld.xml /etc/firewalld/services/promtail.xml

rm -f /etc/promtail/config.yml
tee > /etc/promtail/config.yml << EOF
server:
  http_listen_port: 9080
  grpc_listen_port: 0

positions:
  filename: /tmp/positions.yaml

clients:
  - url: http://localhost:3100/loki/api/v1/push

scrape_configs:
- job_name: system
  static_configs:
  - targets:
      - localhost
    labels:
      job: varlogs
      __path__: /var/log/**/*log

EOF

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
    --web.listen-address=:9000

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
# dnf install WALinuxAgent cloud-init cloud-utils-growpart gdisk hyperv-daemons -y

echo -e "${ANSI_FG_YELLOW}Detecting sensors ...${ANSI_RESET_ALL}"
sensors-detect --auto > /dev/null

echo -e "${ANSI_FG_YELLOW}Enabling services ...${ANSI_RESET_ALL}"
systemctl daemon-reload

systemctl enable --now chronyd
systemctl enable --now pmlogger
# systemctl enable --now waagent
# systemctl enable --now cloud-init
systemctl enable --now prometheus
systemctl enable --now node_exporter
systemctl enable --now cockpit.socket
systemctl enable --now grafana-server
systemctl enable --now loki
systemctl enable --now promtail

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
firewall-cmd --permanent --zone=trusted --add-service=grafana
firewall-cmd --permanent --zone=trusted --add-service=loki
firewall-cmd --permanent --zone=trusted --add-service=promtail
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
echo -e "${ANSI_FG_GREEN}Done!${ANSI_RESET_ALL}"
