from rklasen/fairroot:mini

COPY requirements.txt docker/rc.append /tmp/

RUN apt-get install -y python3-pip \
    && pip3 install --no-cache-dir -r /tmp/requirements.txt \
    && update-alternatives --install /usr/bin/python python /usr/bin/python3.7 2 \
    && cat /tmp/rc.append >> /mnt/work/.bashrc
