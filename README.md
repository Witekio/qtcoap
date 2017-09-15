This repository is not used anymore.

But you can find some files used for the project in the Download section :
- Dockerfile used to create a VNC including the build of the last Qt version (development branch)
- Dockerfile used to run the unit tests with the califormium library

Ubuntu VNC Docker utilisation :
- Build the first time with : sudo docker build -t ubuntu-desktop-lxde-vnc .
- Run with : sudo docker run -it --rm -p 6080:80 -p 5900:5900 -v /var/run/docker.sock:/run/docker.sock -v $(which docker):/bin/docker ubuntu-desktop-lxde-vnc
- Save the state (to avoid to loose modifications when docker close) : 
	1) Know the container id : sudo docker ps -l
	2) Save the container state : sudo docker commit <container_id> ubuntu-desktop-lxde-vnc
- You just need to install QtCreator and clone the qtcoap repository to begin to work

Califormium CoAP Server utilisation :
- Build the first time with : sudo docker build -t coap-server .
- Run with : sudo docker run coap-server

