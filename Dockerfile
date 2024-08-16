FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    libstdc++6 \
    g++ \
    wget \
    libssl-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

COPY src/CTP_API /opt/ctp

# Rename the libraries to the expected names
RUN mv /opt/ctp/_thostmduserapi.so /opt/ctp/libthostmduserapi.so && \
    mv /opt/ctp/_thosttraderapi.so /opt/ctp/libthosttraderapi.so

ENV LD_LIBRARY_PATH=/opt/ctp:$LD_LIBRARY_PATH

WORKDIR /app

COPY src /app

RUN g++ -o trading_app /app/main.cpp /app/CustomMdSpi.cpp /app/CustomTradeSpi.cpp /app/TickToKlineHelper.cpp -L/opt/ctp -lthostmduserapi -lthosttraderapi -lpthread

CMD ["./trading_app"]
