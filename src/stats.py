import os
import matplotlib.pyplot as plt
import random
import time
import paramiko #pip install paramiko

name1 = "tc405-109-16.insa-lyon.fr"

ip = "134.214.202.24"
port = "4314"

def avg(lst):
    return sum(lst)/float(len(lst))

# bouger taille par default de la fenetre,
# alpha et beta du timeout.

def exec(p, name1, name2, port, file_name, pwd):
    client1 = None
    client2 = None
    try:
        print('connection SSH1 a {}'.format(name1))
        client1 = paramiko.SSHClient()
        client1.load_system_host_keys()
        client1.connect(name1, username='mfaucheux1', password=pwd)
        print('./serveur{}-DetecteurML {}'.format(p, port))
        ssh_stdin_s, ssh_stdout_s, ssh_stderr_s = client.exec_command('./serveur{}-DetecteurML {}'.format(p, port))

        print('connection SSH2 a {}'.format(name2))
        client2 = paramiko.SSHClient()
        client2.load_system_host_keys()
        client2.connect(name2, username='mfaucheux1', password=pwd)
        print('./client{} {} {} {}'.format(p, ip, port, file_name))
        ssh_stdin_c, ssh_stdout_c, ssh_stderr_c = ssh.exec_command('./client{} {} {} {}'.format(p, ip, port, file_name))
    finally:
        if client1:
            client1.close()
        if client2:
            client2.close()
    return ssh_stdout_c, ssh_stdout_s

if __name__ == '__main__':
    file_name = str(sys.argv[1])
    pwd = str(sys.argv[2])
    for _ in range(10):
        print(exec(1, name1, name2, port, file_name, pwd)) #921kB sur le client 1 :0 wtfffff bggg, faut juste checker que le fichier est bien reçu (en mode l'image n'est pas corrompue) ah ouii