from rklasen/fairroot:mini

COPY requirements.txt /requirements.txt

RUN apt-get install -y python3-pip

RUN pip3 install -r /requirements.txt

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.7 2