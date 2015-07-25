typedef struct Network Network;

struct Network
{
        int my_socket;
        int (*mqttread) (Network*, unsigned char*, int, int);
        int (*mqttwrite) (Network*, unsigned char*, int, int);
        void (*disconnect) (Network*);
};

void NewNetwork(Network*);
int ConnectNetwork(Network*, char*, int);

