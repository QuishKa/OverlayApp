FROM gcc

RUN apt-get update && \
    apt-get install -y \
    cmake

WORKDIR /app

COPY ./src .

EXPOSE 80

RUN cmake -DCMAKE_BUILD_TYPE=Release . && \
    cmake --build .

ENTRYPOINT [ "./testcmake" ]
    