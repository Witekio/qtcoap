## Qt CoAP internal repository
This is the internal repository used reviews and contributions, before pushing to [Qt public repository](https://codereview.qt-project.org/#/admin/projects/qt/qtcoap)

Resources available in the Download section:

- Dockerfile used to create a VNC including the build of the last Qt version (development branch)
- Dockerfile used to run the unit tests with the califormium library
- Class diagram about the library
- Sequence diagram about a request with the library

### Build and test environment
#### The build and test host

This environment is based on Ubuntu VNC Docker utilisation.

- Build the first time with : sudo docker build -t ubuntu-desktop-lxde-vnc .
- Run with
```bash
sudo docker run -it --rm -p 6080:80 -p 5900:5900 -v /var/run/docker.sock:/run/docker.sock -v $(which docker):/bin/docker ubuntu-desktop-lxde-vnc
```
- Save the state (to avoid to loose modifications when docker close) : 
	1. Know the container id : `sudo docker ps -l`
	2. Save the container state : `sudo docker commit <container_id> ubuntu-desktop-lxde-vnc`
- You just need to install QtCreator and clone the qtcoap repository to begin to work

#### CoAP test server

Test server is based on [Califormium](https://www.eclipse.org/californium/), the Dockerfile can be found in the downloads.

- Build the first time with: `sudo docker build -t coap-server .`
- Run with : `sudo docker run -d -p 5683:5683/udp coap-server`

In order for automatic tests to work, the IP of the test server should be `172.17.0.3`. Docker has no way to give static IP, the easiest is for this container to be the second to start, which should assign it that IP.
You can show containers IP with `docker inspect <container_id>`.

### Links to access the new repository

- Repository access : [https://codereview.qt-project.org/#/admin/projects/qt/qtcoap](https://codereview.qt-project.org/#/admin/projects/qt/qtcoap)
- Review : [https://codereview.qt-project.org/#/c/201311/](https://codereview.qt-project.org/#/c/201311/)

### Other links

- Setting up Gerrit : [https://wiki.qt.io/Setting_up_Gerrit](https://wiki.qt.io/Setting_up_Gerrit)
- Access and update the patch : [https://wiki.qt.io/Gerrit_Introduction#Updating_a_Contribution_With_New_Code](https://wiki.qt.io/Gerrit_Introduction#Updating_a_Contribution_With_New_Code)

## How to use the library

### GET/POST/PUT/DELETE requests
```c++
QCoapClient* client = new QCoapClient(this);
connect(client, &QCoapClient::finished, this, &TestClass::slotFinished);
client->get(QCoapRequest(Qurl("coap://coap.me/test")));
client->put(QCoapRequest(Qurl("coap://coap.me/test")), QByteArray("Some payload"));
```
or
```c++
QCoapReply* reply = client->get(QCoapRequest(QUrl("coap://coap.me/test")));
connect(reply, &QCoapReply::finished, this, &TestClass::slotFinished);
```
The signal finished of the QCoapClient send a pointer to the `QCoapReply` to the slot.
`QCoapReply` objects can be used like `QIODevice` objects.

### OBSERVE requests
The previous way also works for observe request but using the notified signal of the reply can be more useful.
For example :
```c++
QCoapRequest request = QCoapRequest(QUrl("coap://coap.me/obs"));
QCoapReply* reply = client->observe(request);
connect(reply, &QCoapReply::notified, this, &TestClass::slotNotified);
```
and stop the observation with
```c++
client->cancelObserve(request);
```
or
```c++
client->cancelObserve(reply);
```

The notified signal send a `QByteArray` which contains the data of the notification to the slot.

### DISCOVERY requests
```c++
QCoapDiscoveryReply* reply = client->discover(QUrl("coap://coap.me/"));
//QCoapDiscoveryReply works like a QCoapReply but when the signal finished is emitted you can access the list of resources with :
reply->resourceList(); // returns a QList<QCoapResource>
```