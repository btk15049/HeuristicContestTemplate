FROM rust:bullseye

WORKDIR /opt

RUN apt update -y && apt upgrade -y \
    && apt-get install -y \
    g++ \
    git \
    curl \
    wget \
    && apt-get clean

RUN wget https://img.atcoder.jp/ahc029/45e6da0b06.zip \
    && unzip 45e6da0b06.zip \
    && rm 45e6da0b06.zip \
    && cd tools \
    && cargo build --release

RUN apt-get install -y apt-transport-https ca-certificates gnupg \
    && echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] http://packages.cloud.google.com/apt cloud-sdk main" | tee -a /etc/apt/sources.list.d/google-cloud-sdk.list \
    && curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | gpg --dearmor -o /usr/share/keyrings/cloud.google.gpg \
    && apt-get update -y \
    && apt-get install google-cloud-sdk -y

RUN git clone https://github.com/atcoder/ac-library.git \
    && cp -r ac-library/atcoder /usr/include/atcoder

WORKDIR /app

RUN cp /opt/tools/target/release/tester /app/tester
COPY ./task.sh /app/task.sh

CMD bash task.sh
