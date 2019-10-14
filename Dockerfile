FROM novice/build:vc-dc as my_build
WORKDIR /face
COPY app.js package.json ./
RUN npm i && pkg app.js -t node12-linux -o ./app
# build c++ addon
COPY src /face/src
COPY binding.gyp ./
RUN node-gyp configure && node-gyp build
# RUN cmake-js compile


FROM ubuntu:latest
LABEL maintainer="novice <novice@piaoyun.shop>"
WORKDIR /face
# RUN apt-get update && apt-get install -y tzdata 
ENV TZ=Asia/Chongqing
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

COPY --from=my_build /face/app .
COPY --from=my_build /face/build/Release/addon.node .
COPY model/* ./
COPY public ./public

ENTRYPOINT ["./app"]
