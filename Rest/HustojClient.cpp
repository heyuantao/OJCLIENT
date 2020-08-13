//
// Created by he_yu on 2020-01-21.
//
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "HustojClient.h"
#include "Rest.h"
#include "../Common/Utils.h"
#include "../Common/Exceptions.h"
#include "../Common/Logger.h"

using namespace std;

const std::string HustojClient::check_login_api = "/admin/problem_judge.php";
const std::string HustojClient::problem_judge_api = "/admin/problem_judge.php";
//这是新的API接口，将用REST方式来实现
const std::string HustojClient::api_url = "/api/v1/onlinejudge/";


HustojClient::HustojClient(const std::string &base_url){
    this->site_base_url = base_url;
};

HustojClient::~HustojClient() {
}

/*  Not login ,use token for permission
bool HustojClient::login(std::string username, std::string password) {
    this->username = username;
    this->password = password;
}*/
/*
bool HustojClient::checkLogin() {
    std::string url = this->getHttpApiUrl(HustojClient::check_login_api);

    HttpFormData form;
    form.addItem("checklogin","1");

    Response res = this->post(url,form);
    std::string result = res.data;
    if(result.size()==0){
        return false;
    }else{
        return true;
    }
}*/

/**
 * 向API接口发送请求，并获得要判题的一系列题目号的列表
 * POST /{base_api_url}/getjobs/
 * @return
 */
std::vector<std::string> HustojClient::getJobs(){
    std::vector<std::string> job_list;
    try{
        std::string url = this->getHttpApiUrl(HustojClient::api_url)+"jobs/";

        HttpFormData form;
        //form.addItem("getpending","1");
        form.addItem("oj_lang_set","0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17");  //get all kind of problem
        form.addItem("max_running","10");

        Response res = this->post(url,form);
        std::string result = res.data;
        job_list= this->parseResponseString(result);
        return job_list;
    }catch (ClientException &e){
        e.printException();
        std::cout<<"Exception Happen in getJobs !"<<std::endl;
    }
}

Response HustojClient::readCommandOutput(std::string command){
    std::string final_string ="";
    char buffer[512];
    FILE *file=NULL;

    try{
        file = popen(command.c_str(), "r");
    }catch(...){
        throw ClientRunCommandException("Run Command Error !");
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        final_string = final_string+string(buffer);
    }
    int ret = fclose(file);
    bool status = false;
    if(ret==0){
        return Response(final_string, true);
    }else{
        final_string="";
        return Response(final_string, false);
    }
}

/* old version
Response HustojClient::post(std::string url,HttpFormData form_data){
    std::stringstream ss;
    std::string form_data_string = form_data.toFormString();
    //string cmd = "wget --post-data=\""+form_data_string+"\" -q -O - "+"\""+url+"\"";
    ss << boost::format("wget --post-data=\"%s\" -q -O - \"%s\" ") % form_data_string % url << "";
    std::string cmd = ss.str();
    Response res = readCommandOutput(cmd);
    if(res.status== false){
        throw ClientNetworkException(cmd);
    }
    return res;
}
 */

Response HustojClient::post(std::string url,HttpFormData form_data){
    //return WgetRest::post(url,form_data);
    std::stringstream ss;

    try{
        return CasablancaRest::post(url,form_data);
    }catch (ClientUnauthorizedException &e) {
        e.printException();
        ss.str("");
        ss << boost::format("Unauthorized when post \"%s\" with data \"%s\" in HustojClient::post()") % url % form_data.toFormString() << "";
        throw ClientMessageException(ss.str());
    }catch (ClientException &e) {
        e.printException();
        ss.str("");
        ss << boost::format("Exception happen in post \"%s\" with data \"%s\" in HustojClient::post()") % url % form_data.toFormString() << "";
        throw ClientMessageException(ss.str());
    }catch (...){
        ss.str("");
        ss << boost::format("Unknow exception happen in post \"%s\" with data \"%s\" in HustojClient::post()") % url % form_data.toFormString() << "";
        throw ClientMessageException(ss.str());
    }
}


/**
 * POST /{api_url}/updatesolution/
 * @param solution， 表示提交的编号
 * @param result , 0表示正在判题，4表示判题成功
 * @param time
 * @param memory
 * @param pass_rate
 */
void HustojClient::updateSolution(const std::string &solution, int result=0, int time=0, int memory=0, double pass_rate=0.0){
    try{
        std::string url = this->getHttpApiUrl(HustojClient::api_url)+"updatesolution/";

        int sim=0;
        int sim_id=0;
        HttpFormData form;
        //form.addItem("update_solution","1");
        form.addItem("sid",solution);
        form.addItem("result",to_string(result));
        form.addItem("time",to_string(time));
        form.addItem("memory",to_string(memory));
        form.addItem("sim",to_string(sim));
        form.addItem("sim_id",to_string(sim_id));
        form.addItem("pass_rate",to_string(pass_rate));

        Response res = this->post(url,form);

    }catch (ClientException &e){
        e.printException();
        std::cout<<"Exception Happen in updateSolution !"<<std::endl;
    }
}

/**
 * 会抛出异常
 * POST /{api_url}/addcompileerrorinformation/
 * @param solution
 */
void HustojClient::addCompileErrorInformation(const std::string &solution) {
    try{
        std::string compile_error_file = "ce.txt";
        std::ifstream input_stream;
        std::stringstream string_stream;
        input_stream.open(compile_error_file.c_str());
        string_stream << input_stream.rdbuf();
        std::string ce_file_content = string_stream.str();
        input_stream.close();

        std::string url_encode_ce_file_content = this->url_encoder(ce_file_content);

        std::string url = this->getHttpApiUrl(HustojClient::api_url)+"addcompileerrorinformation/";
        HttpFormData form;
        //form.addItem("addceinfo","1");
        form.addItem("sid",solution);
        form.addItem("ceinfo",url_encode_ce_file_content);
        //cout<<form.toFormString();
        Response res = this->post(url,form);

        //return res.status;
    }catch (ClientException &e){
        std::cout<<"Exception Happen in addCompileErrorInformation !"<<std::endl;
        e.printException();
        //return false;
    }
}

/**
 * 会抛出异常
 * POST /{api_url}/getsolution/
 * @param solution
 * @param work_dir
 * @param language
 */
void HustojClient::getSolution(const std::string &solution, std::string work_dir="/tmp", int language=0) {
    try{
        std::string url = this->getHttpApiUrl(HustojClient::api_url)+"getsolution/";
        HttpFormData form;
        //form.addItem("getsolution","1");
        form.addItem("sid",solution);
        Response res = this->post(url,form);

        // Generate the dist source file path
        boost::filesystem::path work_dir_path(work_dir);

        work_dir_path /= string("Main.") + this->getLanguageExtensionById(language);
        std::string filename_path = work_dir_path.string();

        std::ofstream file;
        file.open(filename_path);
        file<<res.data;
        file.close();
        //return res.status;
    }catch (ClientException &e){
        std::cout<<"Exception Happen in getSolution !"<<std::endl;
        e.printException();
    }
}

std::string HustojClient::getHttpApiUrl(const std::string &api) {
    std::stringstream ss;
    std::string url;
    ss << boost::format("%s%s") % this->site_base_url % api << "";
    url = ss.str();
    return url;
}

/**
 * 会抛出异常
 * POST /{api_url}/getsolutioninformation/
 * @param solution
 * @param problem
 * @param username
 * @param lang
 */
void HustojClient::getSolutionInformation(const std::string &solution, std::string &problem, std::string &username, int &lang) {
    try{
        std::string url = this->getHttpApiUrl(HustojClient::problem_judge_api)+"getsolutioninformation/";
        HttpFormData form;
        //form.addItem("getsolutioninfo","1");
        form.addItem("sid",solution);

        Response res = this->post(url,form);
        std::vector<std::string> string_vector = this->parseResponseString(res.data);
        if(string_vector.size()<3){
            std::stringstream ss;
            ss << boost::format("Error in http request:url=%s and form=%s") %url %form.toFormString() <<"";
            throw ClientNetworkException(ss.str());
        }
        //problem = std::stoi(string_vector[0]);
        problem = string_vector[0];
        username = string_vector[1];
        lang = std::stoi(string_vector[2]);
    }catch (ClientException &e){
        e.printException();
        std::stringstream ss ;
        ss << boost::format("Error happen in HustojClient::getSolutionInformation") <<"";
        throw ClientMessageException(ss.str());
    }catch (...){
        std::cout <<"Error in HustojClient::getSolutionInformation"<<std::endl;
    }
}

//Response is split by next line ,parse it into vector<string>
std::vector<std::string> HustojClient::parseResponseString(std::string content) {
    std::vector<std::string> string_vector;
    std::istringstream stream(content);
    for (std::string item; stream >> item; ){
        string_vector.push_back(item);
    }
    return string_vector;
}

void HustojClient::getProblemInformation(const std::string &problem, int &time_limit, int &mem_limit, int &isspj) {
    try{
        std::string url = this->getHttpApiUrl(HustojClient::problem_judge_api);
        HttpFormData form;
        form.addItem("getprobleminfo","1");
        form.addItem("pid",problem);

        Response res = this->post(url,form);

        std::vector<std::string> string_vector = this->parseResponseString(res.data);

        time_limit = atoi(string_vector[0].c_str());
        mem_limit = atoi(string_vector[1].c_str());
        isspj = atoi(string_vector[2].c_str());
        //return res.status;
    }catch (ClientException &e){
        std::cout<<"Exception Happen in getProblemInformation !"<<std::endl;
        e.printException();
    }
}

string HustojClient::url_encoder(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

/**
 * 会抛出异常
 * POST /{api_url}/addruningerrorinformation/
 * @param solution
 */
void HustojClient::addRuningErrorInformation(const std::string &solution) {
    try{
        std::string run_error_file = "error.out";
        std::ifstream input_stream;
        std::stringstream string_stream;
        input_stream.open(run_error_file.c_str());
        string_stream << input_stream.rdbuf();
        std::string re_file_content = string_stream.str();
        input_stream.close();

        std::string url_encode_re_file_content = this->url_encoder(re_file_content);

        std::string url = this->getHttpApiUrl(HustojClient::api_url)+"addruningerrorinformation/";
        HttpFormData form;
        //form.addItem("addreinfo","1");
        form.addItem("sid",solution);
        form.addItem("reinfo",url_encode_re_file_content);
        cout<<form.toFormString();
        Response res = this->post(url,form);

        //return res.status;
    }catch (ClientException &e){
        std::cout<<"Exception Happen in addRuningErrorInformation !"<<std::endl;
        e.printException();
    }
}

void HustojClient::getTestDataList(const std::string &problem, std::vector<std::string> &data_list) {
    try{
        std::string url = this->getHttpApiUrl(HustojClient::problem_judge_api);
        HttpFormData form;
        form.addItem("gettestdatalist","1");
        form.addItem("pid",problem);

        Response res = this->post(url,form);

        std::vector<std::string> string_vector = this->parseResponseString(res.data);
        data_list=string_vector;
        //return res.status;
    }catch (ClientException &e){
        std::cout<<"Exception Happen in getTestDataList !"<<std::endl;
        e.printException();
    }
}

void HustojClient::getTestDataData(const std::string &problem, std::string filename, std::string & filecontent) {
    try{
        std::string url = this->getHttpApiUrl(HustojClient::problem_judge_api);
        std::stringstream stream;
        stream << problem << "/" << filename;
        HttpFormData form;
        form.addItem("gettestdata","1");
        form.addItem("filename",stream.str());
        Response res = this->post(url,form);
        filecontent = res.data;
        //cout << res.status << endl << res.data.size() << endl;
        if(res.data.size()==0){
            throw ClientNetworkException("get zero data in getTestDataData");
        }
        //return res.status;
    }catch (ClientException &e){
        std::cout<<"Exception Happen in getTestDataData !"<<std::endl;
        e.printException();
    }
}

void HustojClient::getTestDataDate(const std::string &problem, std::string filename, std::string &date) {
    try{
        std::string url = this->getHttpApiUrl(HustojClient::problem_judge_api);
        std::stringstream stream;
        stream << problem << "/" << filename;
        HttpFormData form;
        form.addItem("gettestdatadate","1");
        form.addItem("filename",stream.str());
        Response res = this->post(url,form);
        date = res.data;
        if(res.data.size()==0){
            throw ClientNetworkException("get zero data in getTestDataDate");
        }
        //return res.status;
    }catch (ClientException &e){
        std::cout<<"Exception Happen in getTestDataDate !"<<endl;
        e.printException();
    }
}

void HustojClient::getTestFile(const std::string &problem, std::string data_dir) {
    std::vector<std::string> test_file_list;
    std::vector<std::string> test_file_path_string_list;
    std::vector<std::string> download_file_list;

    //create dir
    boost::filesystem::path problem_path(data_dir);
    problem_path /= problem;
    this->createDirectoryIfNotExists(problem_path.string());

    //get test file
    this->getTestDataList(problem,test_file_list);

    for(std::vector<std::string>::iterator item=test_file_list.begin();item!=test_file_list.end();item++){
        boost::filesystem::path file_path(data_dir);
        file_path /= problem;
        file_path /= *item;
        test_file_path_string_list.push_back(file_path.string());
    }

    for(int index =0; index<test_file_list.size() ; index++){
        std::string remote_file = test_file_list[index];
        std::string local_file_path_string = test_file_path_string_list[index];
        std::string remote_file_time_string;
        std::string local_file_time_string;

        //ClientFile localfile(local_file_path);
        boost::filesystem::path local_file_path(local_file_path_string);
        if(!boost::filesystem::exists(local_file_path)){
        //if(localfile.exist()== false){
            download_file_list.push_back(remote_file);
            continue;
        }

        //file exist but is old than server , just pull it
        this->getTestDataDate(problem,remote_file, remote_file_time_string);

        struct stat filestat;
        stat(local_file_path.c_str(), &filestat);
        time_t time= filestat.st_mtime;
        local_file_time_string = std::to_string(time) ;

        //local_file_time_string = localfile.getModifyTime();
        if(stol(local_file_time_string)<stol(remote_file_time_string)){
            download_file_list.push_back(remote_file);
            continue;
        }
    }

    //cout<<"File to Download !"<<endl;
    for(std::vector<std::string>::iterator item=download_file_list.begin();item!=download_file_list.end();item++){
        std::string file_name = *item;
        std::string file_path_string = "";
        std::string file_content = "";

        boost::filesystem::path file_path(data_dir);
        file_path /= problem;
        file_path /= *item;
        file_path_string = file_path.string();

        //cout << file_path_string << endl;
        getTestDataData(problem,file_name,file_content);

        ofstream output_stream;
        output_stream.open(file_path_string);
        output_stream << file_content;
        output_stream.close();
    }
    ClientLogger::DEBUG("Download file finished in HustojClient::getTestFile");

}
/*
string HustojClient::getLanguageExtensionById(int lang) {
    if((lang<0)||(lang>=HustojClient::language_extension.size())){
        std::stringstream ss;
        ss << boost::format("Find lang id %d error in HustojClient::getLanguageExtensionById !") % lang << "";
        throw ClientContainerException(ss.str());
    }
    return HustojClient::language_extension[lang];
}
*/
/*
int HustojClient::getLanguageIdByExtension(string ext) {
    vector<string>::const_iterator itr = std::find(HustojClient::language_extension.begin(), HustojClient::language_extension.end(), ext);
    if(itr==HustojClient::language_extension.end()){
        stringstream ss;
        ss << boost::format("Find lang ext %s error in HustojClient::getLanguageIdByExtension !") % ext << "";
        throw ClientContainerException(ss.str());
    }
    return (itr-HustojClient::language_extension.begin());
}
*/

void HustojClient::createDirectoryIfNotExists(const std::string &dir_path_string) {
    std::string current_path_string = dir_path_string;
    std::vector<std::string> path_string_to_create_list;

    boost::filesystem::path fill_path(current_path_string);
    if(boost::filesystem::exists(fill_path)){
        return;
    }

    // record the directory to be create
    while(true){
        boost::filesystem::path current_path(current_path_string);
        if(!boost::filesystem::exists(current_path)){
            //path_string_to_create_list.push_back(current_path.string());
            path_string_to_create_list.insert(path_string_to_create_list.begin(),current_path.string());
            current_path_string = current_path.parent_path().string();
        }else{
            break;
        }
    }

    //create the directory
    for(std::vector<std::string>::iterator itr = path_string_to_create_list.begin(); itr!= path_string_to_create_list.end(); itr++){
        boost::filesystem::path to_create_path(*itr);
        if(boost::filesystem::exists(to_create_path)){
            continue;
        }else{
            boost::filesystem::create_directories(to_create_path);
        }
    }
}

void HustojClient::updateProblemInformation(const std::string &problem) {
    try{
        std::string url = this->getHttpApiUrl(HustojClient::problem_judge_api);
        std::stringstream stream;

        HttpFormData form;
        form.addItem("updateproblem","1");
        form.addItem("pid",problem);

        Response res = this->post(url,form);
    }catch (ClientException &e){
        std::cout<<"Exception Happen in HustojClient::updateProblemInformation !"<<endl;
        e.printException();
    }
}

void HustojClient::updateUserInformation(const std::string username) {
    try{
        std::string url = this->getHttpApiUrl(HustojClient::problem_judge_api);
        std::stringstream stream;

        HttpFormData form;
        form.addItem("updateuser","1");
        form.addItem("user_id",username);

        Response res = this->post(url,form);
    }catch (ClientException &e){
        std::cout<<"Exception Happen in HustojClient::updateUserInformation() !"<<endl;
        e.printException();
    }
}

void HustojClient::login(const std::string &token) {
    CasablancaRest::api_token=token;
}

bool HustojClient::checkLogin() {
    std::string url = this->getHttpApiUrl(HustojClient::check_login_api);
    HttpFormData form;
    std::string result;

    try{
        form.addItem("checklogin","1");
        Response res = this->post(url,form);
        result = res.data;
        if(result.size()==0){
            return false;
        }else{
            return true;
        }
    }catch (ClientException &e){
        e.printException();
        throw ClientMessageException("Exception happen in HustojClient::checkLogin() ");
    }
}




