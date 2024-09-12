#Buildowanie
docker build -t <image_name> .

#Uruchamianie kontenera
docker run --gpus all --shm-size="8gb" -it --rm -p 8888:8888 -v $(pwd)/ObjectDetection:/home/appuser/ObjectDetection -v $(pwd)/tmp:/tmp:rw <image_name>

#Skopiwać adres jupiter notebook z kontenera dockerowego