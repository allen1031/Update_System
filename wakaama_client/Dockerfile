FROM hypriot/rpi-java

# Install dependencies
RUN apt-get update && apt-get install -y wget\
    python \
    python-dev \
    python-pip \
    python-virtualenv \
	git \
	cmake \
	build-essential \
    --no-install-recommends && \
    rm -rf /var/lib/apt/lists/*

RUN pip install pymongo

VOLUME ["C:\Users\s148340\Desktop\IoT\workplace\Shared"]

ADD ./lwm2mclient ./
ADD ./Client_Main.py ./
ADD ./update-git.sh ./
ADD ./database.py ./
ADD ./Data.py ./
#CMD ["./lwm2mclient","-4","-h","192.168.2.22"]
