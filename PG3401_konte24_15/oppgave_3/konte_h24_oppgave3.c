#define _CRT_SECURE_NO_WARNINGS // Dangerous and we would never do this, except this is a luddite course
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// #include <sys/socket.h>
// #define _BSD_SOURCE
// #include <netdb.h>
// #include <netinet/in.h> // Defines the sockaddr_in on Debian
// #include <unistd.h> // Defines close function on Debian

int ftp_check_user(char *pszUsername, char *pszPassword) {
   int retval = 0;
   FILE* fDatabase;
   char szLine[256];
   char* pszFileUsername;
   char* pszFilePassword;

   if (strcmp(pszUsername, "anonymous") == 0) {
      retval = 1;
   }
   else {
      fDatabase = fopen("user.db", "r");
      if (fDatabase == NULL) {
         perror("Error opening file");
      }
      else {
         while (fgets(szLine, sizeof(szLine), fDatabase)) {
            // Remove newline character if present
            szLine[strcspn(szLine, "\n")] = '\0';

            pszFileUsername = strtok(szLine, ";");
            pszFilePassword = strtok(NULL, ";");

            if (pszFileUsername && pszFilePassword) {
               if (strcmp(pszUsername, pszFileUsername) == 0 && strcmp(pszPassword, pszFilePassword) == 0) {
                  retval = 1;  // Matching username and password found
               }
            }
         }

         fclose(fDatabase);
      }
   }

   return retval;
}
