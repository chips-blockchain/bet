docker build -t bet_armv7l:1.0 -f tools/Dockerfile_armv7l . && docker run -it --mount type=bind,source="$(pwd)",target=/root/Download bet_armv7l:1.0
mkdir -p dist
mv bet-linux-armv7l-$(printf '%s' $(git describe --always)).tar.gz dist/
ls -lh dist/