#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#define PORT 6000

int cek_auth(char data[], char file[]){
    char pathfile[1024];
    sprintf(pathfile, "~/database/%s", file);
    
    FILE *file_akun;
    file_akun = fopen(pathfile, "r");
    if(file_akun == NULL){
        perror("fopen()");
        return EXIT_FAILURE;
    }
    char baris[1024];
    while (fgets(baris, sizeof(baris), file_akun)!=NULL){
        if(strcmp(baris,data)==0){
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]){
    struct sockaddr_in address;
    int server_fd, new_socket, valread, opt = 1, addrlen = sizeof(address);
    // send SERVER> if success gatau kenapa cuma ya gitu aja lah
    char buffer[1024]={0},response[1024]={0};

   //create socket file descriptor
   if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))==0){
       perror("error creating socket");
       exit(EXIT_FAILURE);
   }
   //attaching socket to port
    if (setsockopt(server_fd, SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))< 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // kalo sudo bypass permiss
    char *path = "~/databases/";
    int root;
    char auth_account[1024] = {0};
    char auth_username[1024] = {0};

    recv(new_socket, &root,sizeof(root),0);
    if(!root){
        char auth_response[1024] = {0};
        valread = read(new_socket,auth_account, 1024);
        int cek = cek_auth(auth_account, "account.txt");

        for(int i=0;i<strlen(auth_account);i++){
            if(auth_account[i]==',') break;
            auth_username[i] = auth_account[i];
        }

        if(cek){
            sprintf(auth_response,"Auth berhasil\n");
        } else {
            sprintf(auth_response,"Invalid Credential");
            close(new_socket);
        }
        send(new_socket, auth_response, sizeof(auth_response),0);
    } else {
        strcpy(auth_username, "root");
    }
    //main programm

    while(1){
        // char request[1024] = {0}, response[1024] = {0};
        // valread = read(new_socket, buffer, 1024);
        // printf("%s", request);
        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);
        strcpy(response,"query success\n");
        send(new_socket, response, strlen(response), 0);
        
        char *create_user = strstr(request, "CREATE USER");
        char *create_database = strstr(request, "CREATE DATABASE");
        char *use= strstr(request, "USE");
        char *grant_permission= strstr(request, "GRANT PERMISSION");
        char *create_table= strstr(request, "CREATE TABLE");
        char *drop= strstr(request, "DROP");
        char *insert_into= strstr(request, "INSERT INTO");
        char *select= strstr(request, "SELECT");
        char *delete= strstr(request, "DELETE");
        
         if (create_user){
            if(root){
                char akun[20] = {0};
                int u=0;
                char password[20] = {0};
                int p=0;
                for(int i=12; i<strlen(request); i++){
                    if(request[i] == 32){
                        while(1){
                            i++;
                            if(request[i] == 32 && request[i-1] == 'Y' && request[i-2] == 'B'){
                                i++;
                                akun[u] = ',';
                                u++;
                                break;
                            }
                        }
                    }
                    akun[u] = request[i];
                    u++;
                }
                char path_file_akun[100];
                sprintf(path_file_akun, "databases/client_database/client_account.txt");
                // printf("%s\n", path_file_akun);

                FILE *fakun;
                fakun= fopen(path_file_akun,"a");
                if (fakun == NULL) {
                    perror("fopen()");
                    return EXIT_FAILURE;
                }
                fputs(akun, fakun);
                fputs("\n", fakun);
                fflush(fakun);
                fclose(fakun);
                sprintf(response, "CREATE USER SUCCESS\n");
            }
            else{
                sprintf(response, "ACCESS DENIED!!!\n");
            }
            send(new_socket, response, sizeof(response), 0);
        }

        if (create_database){
            char nama_database[1024]={0};
            char path_database_baru[1024]={0};
            int ii, n=0;
            for(ii=strlen(request); ii>0; ii--){
                if(request[ii] == 32)
                    break;
            }
            for(int i=ii+1; i<strlen(request)-2; i++){
                nama_database[n]=request[i];
                n++;
            }

            //printf("%s\n", nama_database);
            sprintf(path_database_baru, "databases/%s", nama_database);
            int result = mkdir(path_database_baru, 0777);

            if(!root){
                char access_database[1024]={0};
                char username[1024]={0};
                int u=0;
                for(int i=0; i<strlen(auth_akun); i++){
                    if(auth_akun[i]==',')
                        break;
                    username[u]=auth_akun[i];
                    u++;
                }
                sprintf(access_database, "%s,%s", nama_database, username);
                FILE *faccess;
                faccess= fopen("databases/client_database/access_account.txt","a");
                if (faccess == NULL) {
                    perror("fopen()");
                    return EXIT_FAILURE;
                }
                fputs(access_database, faccess);
                fputs("\n", faccess);
                fflush(faccess);
                fclose(faccess);
            }
            database_used[0]='\0';
            strcpy(database_used, nama_database);
            sprintf(response, "CREATE DATABASE SUCCESS\n");
            send(new_socket, response, sizeof(response), 0);
        }

        if (use){
            database_used[0]='\0';
            int ii, n=0;
            for(ii=strlen(request); ii>0; ii--){
                if(request[ii] == 32)
                    break;
            }
            for(int i=ii+1; i<strlen(request)-2; i++){
                database_used[n]=request[i];
                n++;
            }
            char akses[1024];
            char username[1024]={0};
                int u=0;
                for(int i=0; i<strlen(auth_akun); i++){
                    if(auth_akun[i]==',')
                        break;
                    username[u]=auth_akun[i];
                    u++;
                }
            sprintf(akses, "%s,%s", database_used, username);
            int cek = cek_data(akses, "access_account.txt");
            if(cek || root){
                sprintf(response, "%s USED\n", database_used);
            }
            else{
                sprintf(response, "ACCESS DENIED!!!\n");
            }

            send(new_socket, response, sizeof(response), 0);

        }

        if (grant_permission){
            if(root){
                char delim[] = " ";
                char *ptr = strtok(request, delim);
                int pos=0;
                char access[1024]={0};
                while(ptr != NULL)
                {
                    if(pos == 2){
                        strcat(access, ptr);
                        strcat(access, ",");
                    }
                    if(pos == 4)
                        strcat(access, ptr);
                    //printf("'%s'\n", ptr);
                    ptr = strtok(NULL, delim);
                    pos++;
                }

                FILE *faccess;
                faccess= fopen("databases/client_database/access_account.txt","a");
                if (faccess == NULL) {
                    perror("fopen()");
                    return EXIT_FAILURE;
                }
                fputs(access, faccess);
                fputs("\n", faccess);
                fflush(faccess);
                fclose(faccess);
                sprintf(response, "ACCESS GRANTED\n");
            }
            else
                sprintf(response, "ACCESS DENIED!!!\n");
            send(new_socket, response, sizeof(response), 0);
        }

        if (drop) {
            char nama_objek[1024]={0};
            char path_objek[1024]={0};
            char tipe_objek[16]={0};
            char username[1024]={0};
            char akses[1024]={0};
            int result = -1;

            char *delim = " ,;\n";
            char *token = strtok(request, delim);

            for(int i=0; i<strlen(auth_akun); i++)
            {
                if(auth_akun[i]==',')
                    break;
                username[i]=auth_akun[i];
            }

            token = strtok(NULL, delim);
            strcpy(tipe_objek, token);
            if (strcmp(tipe_objek, "DATABASE") == 0)
            {
                token = strtok(NULL, delim);
                strcpy(nama_objek, token);
                sprintf(akses, "%s,%s",nama_objek,username);

            } else if (strcmp(tipe_objek, "TABLE") == 0)
            {
                token = strtok(NULL, delim);
                sprintf(nama_objek, "%s/%s.txt", database_used, token);
                sprintf(akses, "%s,%s",database_used,username);

            }

            sprintf(path_objek, "databases/%s", nama_objek);

            if (cek_data(akses, "access_account.txt") && database_used[0] != '\0')
                result = remove(path_objek);
            if (database_used[0] == '\0')
                sprintf(response, "NO DATABASE IN USE!!!\n");

            if (result==0)sprintf(response, "DROP %s SUCCESS\n", tipe_objek);
            else if (errno==ENOTEMPTY) sprintf(response, "DROP %s FAILED\n%s not empty\n", tipe_objek, nama_objek);
            else if (errno==ENOENT) sprintf(response, "DROP %s FAILED\n%s not exists\n", tipe_objek, nama_objek);
            else sprintf(response, "DROP %s FAILED\n", tipe_objek);

            send(new_socket, response, sizeof(response), 0);
        }

        if (create_table){
            if(database_used[0]=='\0')
                sprintf(response, "NO DATABASE IN USE!!!\n");
            else{
                // printf("%s\n", database_used);
                int spasi=0;
                char table_name[1024]={0};
                char table_details[1024]={0};
                int t=0, td=0;
                for(int i=0; i<strlen(request)-2; i++){
                    if(request[i]=='(' || request[i]==')')
                        continue;
                    if(request[i]==32){
                        spasi++;
                    }
                    if(spasi==2 && request[i]!=32){
                        table_name[t]=request[i];
                        t++;
                    }
                    if(spasi>=3){
                        if((spasi==3 && request[i]==32) || (request[i]==32 && request[i-1]==','))
                            continue;
                        table_details[td]=request[i]; 
                        td++;
                    }
                }
              //  printf("%s %s\n", table_name, table_details);
                char path_file_table[1024]={0};
                sprintf(path_file_table, "databases/%s/%s.txt", database_used, table_name);
                FILE *ftabel;
                ftabel= fopen(path_file_table,"a");
                if (ftabel == NULL) {
                    perror("fopen()");
                    return EXIT_FAILURE;
                }
                fputs(table_details, ftabel);
                fputs("\n\n", ftabel);
                fflush(ftabel);
                fclose(ftabel);

                sprintf(response, "CREATE TABLE SUCCESS\n");     
            }  
            send(new_socket, response, sizeof(response), 0);
        }
        
        if (insert_into){
            //INSERT INTO table1 (�value1�, 2, �value3�, 4);
            if(database_used[0]=='\0')
                sprintf(response, "NO DATABASE IN USE!!!\n");
            else{
                int spasi=0;
                char table_name[1024]={0};
                char data_details[1024]={0};
                int t=0, td=0;
                for(int i=0; i<strlen(request)-2; i++){
                    if(request[i]=='(' || request[i]==')')
                        continue;
                    if(request[i]==32){
                        spasi++;
                    }
                    if(spasi==2 && request[i]!=32){
                        table_name[t]=request[i];
                        t++;
                    }
                    if(spasi>=3){
                        if((spasi==3 && request[i]==32) || (request[i]==32 && request[i-1]==',' || request[i] == '\''))
                            continue;
                        data_details[td]=request[i]; 
                        td++;
                    }
                }
                //printf("%s %s\n", table_name, data_details);

                char path_file_table[1024]={0};
                sprintf(path_file_table, "databases/%s/%s.txt", database_used, table_name);
                FILE *ftabel;
                ftabel= fopen(path_file_table,"r");
                if (ftabel == NULL) {
                    sprintf(response, "TABLE NOT FOUND\n"); 
                }
                else{
                    ftabel= fopen(path_file_table,"a");
                    fputs(data_details, ftabel);
                    fputs("\n", ftabel);
                    fflush(ftabel);
                    fclose(ftabel);
                    sprintf(response, "DATA ADDED SUCCESSFULLY\n"); 
                }
                
            }
            send(new_socket, response, sizeof(response), 0);
        }

        if (select){
            
            if(database_used[0]=='\0')
                sprintf(response, "NO DATABASE IN USE!!!\n");
            else{
                 // SELECT kolom1, kolom2 FROM table1;
                char delim[] = " ";
                request[strlen(request)-2] = '\0';
                // printf("'%s'\n", request);
                char *ptr = strtok(request, delim);
                int pos=0;
                char nama_tabel[1024]={0};
                while(ptr != NULL)
                {
                    if (pos == 1 && strcmp(ptr,"*")==0) {

                    }
                    if (pos == 3) {
                        strcat(nama_tabel, ptr);
                    }
                    ptr = strtok(NULL, delim);
                    pos++;
                }
                char path_file[1024];
                sprintf(path_file, "databases/%s/%s.txt", database_used, nama_tabel);
                printf("%s\n", path_file);

                FILE *ftabel;
                ftabel= fopen(path_file,"r");
                if (ftabel == NULL) {
                    // printf("error openfile");
                    perror("fopen()");
                    return EXIT_FAILURE;
                }
                char line[1024];
                char print_select[2048];
                int num_line = 0;
                while (fgets(line , sizeof(line) , ftabel)!= NULL)
                {
                    if(num_line > 0) {
                        strcat(print_select, line);
                    }
                    num_line++;
                }
                strcpy(response, print_select);
                strcpy(print_select, "");  
            }
            send(new_socket, response, sizeof(response), 0);
                //printf("%s %s\n", table_name, data_details);
        }
        
        if (delete){
            if(database_used[0]=='\0')
                sprintf(response, "NO DATABASE IN USE!!!\n");
            else{
                 // SELECT kolom1, kolom2 FROM table1;
                char delim[] = " ";
                char *ptr = strtok(request, delim);
                int pos=0;
                char nama_tabel[1024]={0};
                while(ptr != NULL)
                {
                    if(pos==2){
                        strcpy(nama_tabel, ptr);
                        nama_tabel[strlen(nama_tabel)-2]='\0';
                    }
                    ptr = strtok(NULL, delim);
                    pos++;
                }
                //printf("nama tabel = %s\n", nama_tabel);
                char path_file[1024];
                sprintf(path_file, "databases/%s/%s.txt", database_used, nama_tabel);

                FILE *ftabel;
                ftabel= fopen(path_file,"r");
                if (ftabel == NULL) 
                    printf("error open file\n");
                    
                char line[1024];
                fgets(line , sizeof(line) , ftabel);
                remove(path_file);
               //printf("%s", line);
                
                ftabel= fopen(path_file,"a");
                if (ftabel == NULL) 
                    printf("error open file\n");
                
                fputs(line, ftabel);
            //  fputs("\n", fakun);
                fflush(ftabel);
                fclose(ftabel);

                sprintf(response, "DATA DELETED SUCCESSFULLY\n");
            }
            send(new_socket, response, sizeof(response), 0);
        }
        char *exit = strstr(request, "exit");
        if(exit)
            break;
    } 
    
    }
    

    return 0;
}


