## Qt CoAP internal repository
This is the internal repository used reviews and contributions, before pushing to [Qt public repository](https://codereview.qt-project.org/#/admin/projects/qt/qtcoap)

To contribute and test, you can use the following docker environments:

- XUbuntu Qt contribution environment: https://hub.docker.com/r/aleravat/qt-contribution-env/
- CoAP test server: https://hub.docker.com/r/aleravat/coap-test-server/

Refer to the associated documentation if needed.

### Setup
#### The build environment

The build environment is provided in `qt-contribution-env` docker.

- Run the docker with the command below.  You can then use a VNC client to connect on 127.0.0.1:5901, or browse to http://127.0.0.1:6901. The default password inherited from the base image is vncpassword.
```bash
docker run -d -p 5901:5901 -p 6901:6901 --privileged aleravat/qt-contribution-env:latest
```

- Install Qt Creator from the online installer already present in your home folder, and clone this repository.

- You can optionally save the docker state at any point:
	1. Know the container id : `sudo docker ps -l`
	2. Save the container state : `sudo docker commit <container_id> my-qt-docker`

### Test server
In order to have automatic tests to work, you need a CoAP test server. The command below will start one from the existing Docker.
```bash
docker run --name coap-test-server -d --rm -p 5683:5683/udp aleravat/coap-test-server:latest
```

For automatic tests to work, the test server should be accessible as with host name "coap-plugtest-server". You should use `docker inspect <container_id>` to get the test server IP, and map the host name to it.
This can be done by:

- Editing `/etc/hosts` (or `C:\Windows\System32\drivers\etc\hosts` on Windows)
- Adding `<container_ip> coap-plugtest-server` to it.

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
QCoapRequest request = QCoapRequest("coap://coap.me/obs");
QCoapReply* reply = client->observe(request);
connect(reply, &QCoapReply::notified, this, &TestClass::slotNotified);
```
and stop the observation with
```c++
client->cancelObserve(reply);
```

The notified signal will provide the QCoapReply and most recent message.

### DISCOVERY requests
```c++
QCoapDiscoveryReply* reply = client->discover("coap://coap.me/");

//QCoapDiscoveryReply works like a QCoapReply but when the signal finished is emitted you can access the list of resources with :
reply->resourceList(); // returns a QList<QCoapResource>
```

## Additional resources

### Links to access the Qt repository

- Repository access : [https://codereview.qt-project.org/#/admin/projects/qt/qtcoap](https://codereview.qt-project.org/#/admin/projects/qt/qtcoap)
- Review : [https://codereview.qt-project.org/#/c/201311/](https://codereview.qt-project.org/#/c/201311/)

### Other links

- Setting up Gerrit : [https://wiki.qt.io/Setting_up_Gerrit](https://wiki.qt.io/Setting_up_Gerrit)
- Access and update the patch : [https://wiki.qt.io/Gerrit_Introduction#Updating_a_Contribution_With_New_Code](https://wiki.qt.io/Gerrit_Introduction#Updating_a_Contribution_With_New_Code)

### Downloads
The following resources are available in the Download section:

- Class diagram about the library
- Sequence diagram about a request with the library

