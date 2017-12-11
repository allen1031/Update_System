
#include "lwm2mclient.h"
#include <libmongoc-1.0/mongoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <assert.h>


int db_connection(char *uri_str, mongoc_collection_t *collection){
	mongoc_client_t *client;
    mongoc_database_t *database;
    
	bson_t *command, reply, *insert;
    bson_error_t error;
	bool retval;
	
	/*
     * Required to initialize libmongoc's internals
     */
    mongoc_init ();
	
	/*
     * Optionally get MongoDB URI from command line
     */
    if (strlen(uri_str)==0) {
		return 1;
    }
	
	/*
     * Create a new client instance
     */
    client = mongoc_client_new (uri_str);
	
	/*
     * Get a handle on the database "db_name" and collection "coll_name"
     */
    database = mongoc_client_get_database (client, "test_database");
    collection = mongoc_client_get_collection (client, "test_database", "update"); 
	
	
	command = BCON_NEW ("ping", BCON_INT32 (1));

	retval = mongoc_client_command_simple (
    client, "admin", command, NULL, &reply, &error);

    if (!retval) {
	   fprintf (stderr, "%s\n", error.message);
	   return EXIT_FAILURE;
    }
	
	bson_destroy (command);
	
	 /*
    * Release our handles and clean up libmongoc
    */
    mongoc_collection_destroy (collection);
    mongoc_database_destroy (database);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
	
	
	return 0;
	
	
}

int doc_query(char *client_url, bson_t *query){
	
	mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    //bson_t *query;
    char *str;
	int n = 0;

    mongoc_init ();

    client = mongoc_client_new (client_url);
    collection = mongoc_client_get_collection (client, "test_database", "update");

    cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);

    while (mongoc_cursor_next (cursor, &doc)) {
	   str = bson_as_canonical_extended_json (doc, NULL);
	   if (strlen(str)!=0)
		   n++;
	   fprintf (stdout,"%s\n", str);
	   bson_free (str);
    }
	
    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
	
	if(n == 0){
		return 1;
	}
   
    return 0;
}


int doc_insert(char *client_url, bson_t* doc){
	
	mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_oid_t oid;
	int ret;
    //bson_t *doc;

    mongoc_init ();

    client = mongoc_client_new (client_url);
    collection = mongoc_client_get_collection (client, "test_database", "update");

    if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
        fprintf (stderr, "%s\n", error.message);
		ret = 1;
    }

    bson_destroy (doc);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
    mongoc_cleanup ();

    return ret;
	
}

int doc_update(char *update_uri, bson_t* query, bson_t* update){
	
	int ret=0;
	mongoc_collection_t *collection;
	mongoc_client_t *client;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc = NULL;

	mongoc_init ();

	client = mongoc_client_new (update_uri);
	collection = mongoc_client_get_collection (client, "test_database", "update");

	if (!mongoc_collection_update (
		collection, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
	    fprintf (stderr, "%s\n", error.message);
	    ret = 1;
	  goto fail;
	}

fail:
	if (doc)
	  bson_destroy (doc);
	if (query)
	  bson_destroy (query);
	if (update)
	  bson_destroy (update);
	

	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);
	mongoc_cleanup ();

	return ret;
	
}

int doc_delete(char *delete_uri, bson_t* query){
	
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;
	int ret=0;

	mongoc_init ();

	client = mongoc_client_new (delete_uri);
	collection = mongoc_client_get_collection (client, "test_database", "update");

	if (!mongoc_collection_remove (
		  collection, MONGOC_REMOVE_SINGLE_REMOVE, query, NULL, &error)) {
	  fprintf (stderr, "Delete failed: %s\n", error.message);
	  ret = 1;
	}

	bson_destroy (query);
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);
	mongoc_cleanup ();

	return ret;
}

int doc_count(char *count_uri, bson_t* query){
	
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;
	//bson_t *doc;
	int64_t count;
	int ret = 0;

	mongoc_init ();

	client = mongoc_client_new (count_uri);
	collection = mongoc_client_get_collection (client, "test_database", "update");

	count = mongoc_collection_count (
	  collection, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);

	if (count < 0) {
	  fprintf (stderr, "%s\n", error.message);
	  ret = 1;
	} else {
	  printf ("%" PRId64 "\n", count);
	}

	bson_destroy (query);
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);
	mongoc_cleanup ();

	return 0;
	
}

int doc_read_str(char *read_uri, bson_t* query, char *key, char* value){
	
	mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    //bson_t *query;
    char *str;
	int n = 0;
	
	bson_iter_t iter;
	bson_iter_t iter2;
	uint32_t length = 32;

    mongoc_init ();

    client = mongoc_client_new (read_uri);
    collection = mongoc_client_get_collection (client, "test_database", "update");

    cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);

    while (mongoc_cursor_next (cursor, &doc)) {
	   str = bson_as_canonical_extended_json (doc, NULL);
	   if (strlen(str)!=0)
		   n++;
	   //fprintf (stdout,"%s\n", str);
	   bson_free (str);
	   
	   bson_iter_init(&iter,doc);
	   bson_iter_find_descendant(&iter,key,&iter2);
	   
	   snprintf(value,length,"%s\n",bson_iter_utf8(&iter2,&length));
	   printf("value is %s\n",value);
    }
	
	//printf("%s\n",bson_iter_utf8(&iter2,&length));
	
    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
	
	if(n == 0){
		return 1;
	}
   
    return 0;
	
}

int doc_read_int(char *read_uri, bson_t* query, char *key, int32_t value){
	
	mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    //bson_t *query;
    char *str;
	int n = 0;
	
	bson_iter_t iter;
	bson_iter_t iter2;
	//uint32_t length = 32;

    mongoc_init ();

    client = mongoc_client_new (read_uri);
    collection = mongoc_client_get_collection (client, "test_database", "update");

    cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
	//printf("searching cursor\n");
    while (mongoc_cursor_next (cursor, &doc)) {
	   str = bson_as_canonical_extended_json (doc, NULL);
	   if (strlen(str)!=0)
	  	   n++;
	   //fprintf (stdout,"%s\n", str);
	   bson_free (str);
	   
	   bson_iter_init(&iter,doc);
	   bson_iter_find_descendant(&iter,key,&iter2);
	   
	   value = bson_iter_int32(&iter2);
	   printf("value is %d\n",value);
    }
	
	//printf("%s\n",bson_iter_utf8(&iter2,&length));
	
    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
	
	if(n == 0){
		return 1;
	}
   
    return 0;
	
}


int doc_read_double(char *read_uri, bson_t* query, char *key, double value){
	
	mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    //bson_t *query;
    char *str;
	int n = 0;
	
	bson_iter_t iter;
	bson_iter_t iter2;
	//uint32_t length = 32;

    mongoc_init ();

    client = mongoc_client_new (read_uri);
    collection = mongoc_client_get_collection (client, "test_database", "update");

    cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
	//printf("searching cursor\n");
    while (mongoc_cursor_next (cursor, &doc)) {
	   str = bson_as_canonical_extended_json (doc, NULL);
	   if (strlen(str)!=0)
	  	   n++;
	   //fprintf (stdout,"%s\n", str);
	   bson_free (str);
	   
	   bson_iter_init(&iter,doc);
	   bson_iter_find_descendant(&iter,key,&iter2);
	   
	   value = bson_iter_double(&iter2);
	   printf("value is %d\n",value);
    }
	
	//printf("%s\n",bson_iter_utf8(&iter2,&length));
	
    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
	
	if(n == 0){
		return 1;
	}
   
    return 0;
	
}

int JSON2BSON(const char *json){
	
		bson_error_t error;
		bson_t      *bson;
		char        *string;

		//const char *json =" { "name" : { "first" : "Grace", "last" : "Hopper" }} ";
		printf("%s\n",json);
		bson = bson_new_from_json ((const uint8_t *)json, -1, &error);

		if (!bson) {
		fprintf (stderr, "%s\n", error.message);
		return EXIT_FAILURE;
		}

		string = bson_as_canonical_extended_json (bson, NULL);
		printf ("%s\n", string);
		bson_free (string);

		return 0;
	
}



