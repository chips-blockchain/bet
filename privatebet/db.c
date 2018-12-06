#include <stdio.h>
#include<stdlib.h>
#include <sqlite3.h> 

char *LN_db="../../.chipsln/lightningd.sqlite3";
int32_t LN_get_channel_status(char *id)
{
	  sqlite3 *db;
  	  sqlite3_stmt *stmt = NULL;	
  	  char *err_msg = 0,*sql=NULL;
        int rc;
	  	
     rc = sqlite3_open(LN_db, &db);
       if( rc ) {
             fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
                return(0);
        } else {
              fprintf(stderr, "Opened database successfully\n");
         }
       	#if 0
    	 sql="select * from peers where lower(hex(node_id))=?";
 	 if (rc != SQLITE_OK) {
 	    printf("Failed to prepare statement: %s\n\r", sqlite3_errstr(rc));
       	sqlite3_close(db);
   	return 0;
	}	 
 	else {
    	printf("SQL statement prepared: OK\n\n\r");
	}
	rc = sqlite3_bind_int(stmt, 1, id);
	if (rc != SQLITE_OK) {
		printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
		sqlite3_close(db);
		return 1;
	} 
	else {
	printf("SQL bind integer param: OK\n\n\r");
	}
	rc = sqlite3_step(stmt);
	 if (rc == SQLITE_ROW) {
	    printf("Peer ID:%s\n", sqlite3_column_text(stmt, 0));
	  }
	#endif
 	return 1;    
}


int main(int argc, char* argv[]) {
      /*sqlite3 *db;
      char *err_msg = 0,*sql=NULL;
      int rc;
      const char* data = "Callback function called";
      sqlite3_stmt *res;	
    rc = sqlite3_open("../../.chipsln/lightningd.sqlite3", &db);
    printf("\nrc=%d",rc);
       if( rc ) {
          fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
           return(0);
      } else {
	      fprintf(stderr, "Opened database successfully\n");
     }
     sql = "SELECT * from peers";
	rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

     
     
     if( rc != SQLITE_OK ) {
	         fprintf(stderr, "SQL error: %s\n", err_msg);
	       sqlite3_free(err_msg);
	   } else {
	        fprintf(stdout, "Operation done successfully\n");
	   }
	   printf("%s\n", sqlite3_libversion()); 
      sqlite3_close(db);*/
	LN_get_channel_status(NULL);
}
#if 0

int callback(void *NotUsed, int argc, char **argv, 
		                    char **azColName) {
	    
      NotUsed = 0;
      for (int i = 0; i < argc; i++) {
         printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
       }
       printf("\n");
       return 0;
}
#endif
