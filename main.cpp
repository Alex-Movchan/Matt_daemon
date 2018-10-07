
#include "Daemon.h"

Daemon*    daemon1;

void    signhendl(int sidn)
{
        daemon1->SignalsHandler(sidn);
}

int     main(void)
{
        daemon1 = new Daemon();
        signal(SIGTERM, signhendl);
        daemon1->Run();
        delete daemon1;
}