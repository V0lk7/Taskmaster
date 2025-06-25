FROM ubuntu:24.04

RUN apt update && apt install -y build-essential clang cmake git libreadline-dev zsh curl rsyslog valgrind systemctl

RUN systemctl start rsyslog

RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100 && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100 && update-alternatives --set cc /usr/bin/clang && update-alternatives --set c++ /usr/bin/clang++

WORKDIR /app/taskmaster

CMD ["sh", "-c", "while true; do sleep 3600; done"]
