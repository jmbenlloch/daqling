FROM gitlab-registry.cern.ch/ci-tools/ci-worker:cc7

RUN yum install -y ansible
COPY daqling daqling
WORKDIR daqling
RUN source cmake/setup.sh
RUN ansible-playbook ansible/set-up-host.yml --tags=minimal
RUN yum install -y supervisor
COPY daqling/ansible/roles/supervisor/files/etc/supervisord.d/web-gui.ini /etc/supervisord.d/
RUN yum install -y python-pip
RUN pip install supervisor_twiddler
COPY daqling/ansible/roles/twiddler/files/etc/twiddler.ini /etc/supervisord.d/
RUN mkdir /log
CMD ["/usr/bin/supervisord']
