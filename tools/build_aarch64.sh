docker build -t bet_aarch64:1.0 -f tools/Dockerfile_aarch64 . && docker run -it --mount type=bind,source="$(pwd)",target=/root/Download bet_aarch64:1.0
mkdir -p dist
mv bet-linux-aarch64-$(printf '%s' $(git describe --always)).tar.gz dist/
ls -lh dist/