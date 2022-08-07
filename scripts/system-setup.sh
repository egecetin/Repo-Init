#!/bin/bash

# Color Variables
ANSI_FG_BLACK="\x1b[30m"
ANSI_FG_RED="\x1b[31m"
ANSI_FG_GREEN="\x1b[32m"
ANSI_FG_YELLOW="\x1b[33m"
ANSI_FG_BLUE="\x1b[34m"
ANSI_FG_MAGENTA="\x1b[35m"
ANSI_FG_CYAN="\x1b[36m"
ANSI_RESET_ALL="\x1b[0m"

echo -e "${ANSI_FG_YELLOW}Installing packages ...${ANSI_RESET_ALL}"
yum install epel-release -y
yum install htop cockpit cockpit-pcp chrony mlocate lm_sensors smartmontools -y
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
mv -f scripts/data/neofetch_config.conf /root/.config/neofetch/config.conf
echo "neofetch" >> /etc/profile.d/neofetch-init.sh

# echo -e "${ANSI_FG_YELLOW}Installing IDRAC tools ...${ANSI_RESET_ALL}"
# curl -O https://linux.dell.com/repo/hardware/dsu/bootstrap.cgi
# bash bootstrap.cgi
# rm -f bootstrap.cgi
# yum install dell-system-update -y
# yum install srvadmin-* -y

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
mv /tmp/oneAPI.repo /etc/yum.repos.d
yum install intel-basekit -y

# echo -e "${ANSI_FG_YELLOW}Installing Hyper-V tools"
# yum install -y WALinuxAgent cloud-init cloud-utils-growpart gdisk hyperv-daemons

# Add config for VS Code
mv -f scripts/data/cmake-tools-kits.json /root/.local/share/CMakeTools/

echo -e "${ANSI_FG_YELLOW}Detecting sensors ...${ANSI_RESET_ALL}"
sensors-detect --auto > /dev/null

echo -e "${ANSI_FG_YELLOW}Enabling services ...${ANSI_RESET_ALL}"
/sbin/chkconfig ipmi on
systemctl start ipmi
systemctl enable cockpit.socket
systemctl start cockpit.socket
systemctl enable pmlogger
systemctl start pmlogger

systemctl enable chronyd
systemctl start chronyd

# systemctl enable waagent
# systemctl start waagent
# systemctl enable cloud-init
# systemctl start cloud-init

systemctl stop dnf-makecache.timer
systemctl disable dnf-makecache.timer

echo -e "${ANSI_FG_YELLOW}Disabling non-secure ciphers ...${ANSI_RESET_ALL}"
echo "" >> /etc/ssh/sshd_config
echo "# Custom ciphers" >> /etc/ssh/sshd_config
echo "Ciphers aes128-ctr,aes192-ctr,aes256-ctr" >> /etc/ssh/sshd_config
echo "MACs hmac-sha1,umac-64@openssh.com" >> /etc/ssh/sshd_config
echo -e "${ANSI_FG_YELLOW}Restarting sshd service ...${ANSI_RESET_ALL}"
systemctl restart sshd.service

echo -e "${ANSI_FG_YELLOW}Configuring firewall ...${ANSI_RESET_ALL}"
firewall-cmd --permanent --zone=public --add-service=ssh
firewall-cmd --permanent --zone=public --add-service=cockpit
firewall-cmd --reload

echo -e "${ANSI_FG_YELLOW}Displaying information ...${ANSI_RESET_ALL}"
hostnamectl
echo ""
echo ""
chronyc sources -v
echo ""
echo ""
ip addr
echo ""
echo ""

echo "Done!"
