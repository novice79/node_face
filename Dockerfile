FROM novice/build:dc as my_build
WORKDIR /workspace
RUN mkdir deps dist
COPY app.js package.json ./
RUN npm i && pkg app.js -t node10-linux -o ./dist/app
# build c++ addon
COPY src ./src
COPY CMakeLists.txt ./
RUN cmake-js compile
# copy .so symlink & target files together
RUN ldd build/Release/addon.node \
| grep "=> /" \
| awk '{print $3}' \
| sort \
| uniq \
| xargs -I '{}' sh -c 'cp --parents `readlink -f {}` ./deps ; cp --parents -P {} ./deps' \
&& cp build/Release/addon.node ./dist

FROM ubuntu:latest
LABEL maintainer="novice <novice@piaoyun.shop>"
WORKDIR /face
# RUN apt-get update && apt-get install -y tzdata 
ENV TZ=Asia/Chongqing
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

COPY --from=my_build /workspace/deps /
COPY --from=my_build /workspace/dist ./
COPY model/* ./
COPY public ./public
RUN export LD_LIBRARY_PATH=/usr/local/lib && ldconfig


ENTRYPOINT ["./app"]
