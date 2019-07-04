while true
do
if ! pgrep -x "bet" > /dev/null
then
    ./bet dcv 127.0.0.1
fi
done
