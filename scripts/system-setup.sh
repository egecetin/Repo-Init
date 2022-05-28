#!/bin/bash

echo "Installing packages ..."
yum install epel-release -y
yum install htop cockpit cockpit-pcp chrony mlocate lm_sensors smartmontools -y
yum install OpenIPMI ipmitool -y

echo "Installing cockpit-navigator ..."
curl -sSL https://repo.45drives.com/setup | sudo bash
yum install cockpit-navigator -y

echo "Installing neofetch ..."
wget https://github.com/dylanaraps/neofetch/archive/refs/tags/7.1.0.tar.gz
tar -xzvf 7.1.0.tar.gz neofetch-7.1.0/
make install -C neofetch-7.1.0
rm -rf neofetch-7.1.0
mkdir -p /root/.config/neofetch
mv -f scripts/data/neofetch_config.conf /root/.config/neofetch/config.conf
echo "neofetch" >> /etc/profile.d/neofetch-init.sh

echo "Installing IDRAC tools ..."
curl -O https://linux.dell.com/repo/hardware/dsu/bootstrap.cgi
bash bootstrap.cgi
rm -f bootstrap.cgi
yum install dell-system-update -y
yum install srvadmin-* -y

echo "Installing Intel oneAPI ..."
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

echo "Detecting sensors ..."
sensors-detect --auto > /dev/null

echo "Enabling services ..."
/sbin/chkconfig ipmi on
systemctl start ipmi
systemctl enable cockpit.socket
systemctl start cockpit.socket
systemctl enable pmlogger
systemctl start pmlogger

systemctl stop dnf-makecache.timer
systemctl disable dnf-makecache.timer

echo "Disabling non-secure ciphers ..."
echo "" >> /etc/ssh/sshd_config
echo "# Custom ciphers" >> /etc/ssh/sshd_config
echo "Ciphers aes128-ctr,aes192-ctr,aes256-ctr" >> /etc/ssh/sshd_config
echo "MACs hmac-sha1,umac-64@openssh.com" >> /etc/ssh/sshd_config
echo "Restarting sshd service ..."
systemctl restart sshd.service

echo "Configuring firewall ..."
firewall-cmd --permanent --zone=public --add-service=ssh
firewall-cmd --permanent --zone=public --add-service=cockpit
firewall-cmd --reload

echo "Done!"
