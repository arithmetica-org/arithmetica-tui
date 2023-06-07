#!/bin/bash

set -e

# Set default command to download archive from
DOWNLOAD_CMD="wget -q"

# Set default install prefix to /usr
PREFIX="${PREFIX:-/usr}"

# Check if the user is root
if [ "$(id -u)" -ne 0 ]
then
  echo -e "\e[31mPlease run as root!\e[0m" > /dev/stderr
  exit
fi

# Check for unzip and wget
echo -e "Testing for unzip..."
if ! command -v unzip &> /dev/null
then
  echo -e "\e[31munzip not installed!\e[0m" > /dev/stderr
  exit
fi
echo -e "Testing for wget..."
if ! command -v wget &> /dev/null
then
  echo -e "\e[31mwget not installed, defaulting to curl!\e[0m"  > /dev/stderr
  DOWNLOAD_CMD="curl -O -L -J -s"
  if ! command -v curl &> /dev/null
  then
    echo -e "\e[31mcurl not installed!\e[0m"  > /dev/stderr
    exit
  fi
fi

cd /tmp
# If the directory exists, delete it
if [ -d "arithmetica-tui-install" ]; then
  rm -rf arithmetica-tui-install
fi
mkdir arithmetica-tui-install
cd arithmetica-tui-install
# Download the latest release
$DOWNLOAD_CMD https://github.com/avighnac/arithmetica-tui/releases/latest/download/arithmetica.out

echo "Sucessfully downloaded the latest release."

# Copy the executable to /usr/bin/arithmetica
chmod +x arithmetica.out
cp arithmetica.out $PREFIX/bin/arithmetica

echo "Copied successfully!"

# Delete the temporary directory
cd ..
rm -rf arithmetica-tui-install

exit 0