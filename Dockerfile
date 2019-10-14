FROM novice/build:vc-dc as my_build
WORKDIR /face
# build c++ addon
COPY src /face/src
COPY binding.gyp ./
RUN node-gyp configure && node-gyp build
# RUN cmake-js compile


FROM node:10
LABEL maintainer="novice <novice@piaoyun.shop>"
WORKDIR /face
# RUN apt-get update && apt-get install -y tzdata 
ENV TZ=Asia/Chongqing
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
COPY app.js package.json ./
RUN npm i 
COPY --from=my_build /face/build/Release/addon.node .
COPY model/* ./
COPY public ./public

ENTRYPOINT ["node ./app.js"]
