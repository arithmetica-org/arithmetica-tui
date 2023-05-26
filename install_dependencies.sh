if [ "$EUID" -ne 0 ]
  then echo "Please run as root!"
  exit
fi

curl -s -H "Accept: application/vnd.github.v3.raw" https://api.github.com/repos/avighnac/arithmetica/contents/install.sh | sudo bash
curl -s -H "Accept: application/vnd.github.v3.raw" https://api.github.com/repos/avighnac/basic_math_operations/contents/install.sh | sudo bash