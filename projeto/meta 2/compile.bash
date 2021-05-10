gcc -W -Wall -Wextra -pedantic -Werror -g fileIO.c tcp_server.c -o TcpServer
gcc -W -Wall -Wextra -pedantic -Werror -g menus.c fileIO.c tcp_client.c -o TcpClient
gcc -W -Wall -Wextra -pedantic -Werror -g fileIO.c udp_server.c -o UdpServer
gcc -W -Wall -Wextra -pedantic -Werror -g menus.c fileIO.c udp_client.c -o UdpClient