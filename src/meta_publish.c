#include <errno.h>
#include "mio.h"
#include "libmio_xml.h"


#define USAGE "./meta_copy -id [node_id] -u [username] -p [password] " \
  "-path [path to xml file] -node [node to copy from] " \
  "-useform (interactive parameter setting) -usecsv [path to csv file with parameters] " \
  "-propery_name [comma separated property names] -property_value [comma separated property values]\n"

typedef struct {
    char* node_id;
    char* path;
    char* node;
    int interactive;
    int overwrite;
    char* csv_path;
    char* username;
    char* password;
    char* property_names;
    char* property_values;
} args_t;

args_t* _parse_args(int argc, char **argv)
{
    int i;
    args_t *args = malloc(sizeof(args_t));
    memset(args,0x0,sizeof(args_t));
    for (i = 1; i < argc; i+=2)
    {
        if (strcmp(argv[i],"-path") == 0)
        {
            args -> path = argv[i+1];
        } else if (strcmp(argv[i],"-id") == 0)
            args -> node_id = argv[i+1];
        else if (strcmp(argv[i],"-u") == 0)
            args -> username = argv[i+1];
        else if (strcmp(argv[i],"-p") == 0)
            args -> password = argv[i+1];
        else if (strcmp(argv[i],"-node") == 0)
            args -> node = argv[i+1];
        else if (strcmp(argv[i],"-useform") == 0)
        {
            i--;
            args -> interactive = 1;
        }
        else if (strcmp(argv[i],"-usecsv") == 0)
            args -> csv_path = argv[i+1];
        else if (strcmp(argv[i],"-overwrite") == 0)
        {
            i--;
            args -> overwrite = 1;
        }
    }
    if (args -> node_id == NULL)
    {
        printf("event node id (-id) required\n");
        free(args);
        args = NULL;
    } else if (args -> username == NULL)
    {
        printf("username (-u) required\n");
        free(args);
        args = NULL;
    } else if (args -> password == NULL)
    {
        printf("password (-p) required\n");
        free(args);
        args = NULL;
    }
    return args;
}

static int32_t local_parse_xml(char* xml_file,void (*start_handler)(void *data, const char *element, const char **attribute),
                         void (*end_handler)(void *data, const char *el),  XML_CharacterDataHandler
                         data_handler, void* parser_data)
{
    int32_t buff_size = 512, err, done = 0, break_out = 0;
    char buff[512], *ret;
    FILE *fp;
    fp = fopen(xml_file, "r");
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, start_handler, end_handler);
    XML_SetCharacterDataHandler(parser, data_handler);
    XML_SetUserData(parser, parser_data);
    ((mio_xml_parser_data_t*)parser_data)->parser = &parser;

    while (!done)
    {
        ret = fgets(buff, buff_size, fp);
        if (ret == NULL)
            break;
        if (feof(fp))
        {
            done = 1;
            break_out = 1;
        }
        if (XML_STATUS_ERROR == (XML_Parse(parser, buff, strlen(buff), done))) {
            err = errno;
            XML_ParserFree(parser);
            printf("XML_Parse Failed %s\n", XML_ErrorString(err));
            return 1;
        }
        if (break_out)
        {
            printf("BROKe\n");
            break;
        }
    }
    //XML_ParserFree(parser);
    fclose(fp);
    return 0;
}

int main(int argc, char **argv)
{
    int err;
    args_t *args;
    mio_conn_t *conn;
    mio_stanza_t *meta_item;
    mio_response_t *merge_response, *meta_src_response;
    mio_meta_t *meta_src;
    mio_packet_t *packet;
    mio_xml_parser_data_t *mio_data;
    args = _parse_args(argc, argv);
    if (args == NULL)
    {
        printf(USAGE);
        return 1;
    }

    conn = mio_conn_new(MIO_LEVEL_DEBUG);
    err = mio_connect(args -> username, args -> password, NULL, NULL, conn);
    if (err != MIO_OK) {

        free(args);
        return err;
    }

    meta_src_response = mio_response_new();
    if (args -> path != NULL)
    {
        meta_src_response -> response = mio_packet_new();
        ( (mio_packet_t*) meta_src_response -> response) -> payload = mio_meta_new();
        mio_data = mio_xml_parser_data_new();
        mio_data ->response = meta_src_response;
        err = mio_xml_parse(conn,args->path, xml_data, mio_XMLstart_pubsub_meta_receive,
        		mio_XMLString_geoloc);
        //err = local_parse_xml(args->path, mio_XMLstart_pubsub_meta_receive,
         //               endElement, mio_XMLString_geoloc, mio_data);
    } else
    {
        err = mio_meta_query(conn,args->node,meta_src_response);
    }


    packet = (mio_packet_t*) meta_src_response->response;
    if (packet->type != MIO_PACKET_META)
    {
        mio_response_print(meta_src_response);
        return MIO_ERROR_UNEXPECTED_RESPONSE;
    }
    meta_src = (mio_meta_t*) packet->payload;

    //mio_response_print(meta_src_response);
    merge_response = mio_response_new();
    if (args -> overwrite) { 
        err = mio_meta_publish(conn,args->node_id,meta_src, merge_response);
	//meta_item = mio_meta_to_item(conn, meta_src);
	//mio_item_publish(conn,meta_item, args->node_id, merge_response);
    } else
        err = mio_meta_merge_publish(conn, args->node_id, meta_src, meta_src -> transducers,
                                     meta_src -> properties, merge_response);

    if (err != MIO_OK)
    {
        mio_response_print(merge_response);
        return 1;
    }

    mio_response_free(meta_src_response);
    mio_response_free(merge_response);
    mio_disconnect(conn);
    mio_conn_free(conn);
    free(args);
    return 0;
}


