from rklasen/fairroot:miniApr21p2

ARG USERNAME=pandauser
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# we're inherting from faoroot, in which we are UID 1000
USER root

COPY requirements.txt docker/rc.append /tmp/

RUN pip3 install --no-cache-dir -r /tmp/requirements.txt \
    && update-alternatives --install /usr/bin/python python /usr/bin/python3.7 2 \
    && cat /tmp/rc.append >> /mnt/work/.bashrc

USER $USERNAME