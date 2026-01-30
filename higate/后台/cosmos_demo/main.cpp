#include <iostream>
#include <fstream>
#include <string>
#include <librdkafka/rdkafkacpp.h>
#include "subcenter.pb.h"
#include "rpc.pb.h"

using namespace std;

int m_sub_push_count = 0;
std::set<std::string> m_set_push_topic;// = {"topic_plugin_cosmos"};

map<string, string> MapConfigFile;	//配置缓存

RdKafka::Producer* m_producer = NULL;

using google::protobuf::Message;
using message_ptr       = std::shared_ptr<Message>;


/**
 * 读取配置文件tool.ini，解析其中以key=value格式的内容，并存储到MapConfigFile中。
 * 配置文件中的行如果包含“#”符号，则忽略该行。
 * 配置项必须符合key=value格式，否则忽略该行。
 */
void ReadConfigFile()
{
    // 打印提示信息
    printf("读取配置如下,必须是key=value的格式,其中如包含“#”符号，则跳过。\n");
    
    // 配置文件名称
    string strFileName = "tool.ini";
    // 打开配置文件
    std::ifstream ifs(strFileName.c_str(), std::fstream::in);
    if (ifs.is_open())
    {
        char buf[20000];   // 用于存储每一行内容的缓冲区
        
        // 逐行读取文件内容
        while (ifs.getline(buf, 20000))
        {
            string strLine = buf;    // 将当前行内容转换为string
            // 检查行内容是否包含#符号，如果是则跳过
            if (strLine.find("#") != string::npos)
                continue;

            // 查找等号的位置
            int nIndex = strLine.find("=");
            if (nIndex == string::npos)
                continue;   // 如果没有找到等号，跳过该行

            // 分割key和value
            string strLeft = strLine.substr(0, nIndex);          // key部分
            string strRight = strLine.substr(nIndex + 1, strLine.length() - nIndex - 2);  // value部分

            // 将key-value对存储到MapConfigFile中
            MapConfigFile[strLeft] = strRight;

            // 输出读取结果
            printf("%s=%s\n", strLeft.c_str(), strRight.c_str());
        }
    }
    ifs.close();   // 关闭文件流
}

vector<string> splitString(const string& str) {
    vector<string> result;
    size_t start = 0;
    size_t end = str.find(';');

    while (end != string::npos) {
        string token = str.substr(start, end - start);
        result.push_back(token);
        start = end + 1;
        end = str.find(';', start);
    }

    // 添加最后一个token（如果有的话）
    if (start < str.length()) {
        string token = str.substr(start);
        result.push_back(token);
    }

    return result;
}

// 序列化与反序列化
std::string protobuf_encode(const google::protobuf::Message& message)
{
    std::string        result;
    const std::string& typeName = message.GetTypeName();
    auto               nameLen  = static_cast<int32_t>(typeName.size() + 1);
    result.append(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
    result.append(typeName.c_str(), nameLen);
    if (!message.AppendToString(&result))
    {
        result.clear();
    }
    return result;
}

google::protobuf::Message* create_pb_message(const std::string& type_name)
{
    google::protobuf::Message*                            message = nullptr;
    const google::protobuf::Descriptor* descriptor
        = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (descriptor)
    {
        const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype)
        {
            message = prototype->New();
        }
    }
    return message;
}

message_ptr protobuf_decode(const std::string& buf)
{
    message_ptr result   = nullptr;
    auto        len      = static_cast<int32_t>(buf.size());
    int32_t     nInitLen = sizeof(int32_t);

    const char* begin = buf.c_str();

    int32_t nameLen;
    memcpy(&nameLen, begin, nInitLen);
    if (nameLen <= len - nInitLen + 1 && nameLen > 1)
    {
        std::string typeName(buf.begin() + nInitLen, buf.begin() + nInitLen + nameLen - 1);
        google::protobuf::Message*    message = create_pb_message(typeName);
        if (message)
        {
            const char* data    = buf.c_str() + nInitLen + nameLen;
            int32_t     dataLen = len - nameLen - nInitLen;
            if (dataLen > 0)
            {
                if (message->ParseFromArray(data, dataLen))
                {
                    result = message_ptr(message);
                }
                else
                {
                    if (dataLen - 1 > 0)
                    {
                        // 兼容多传输一个字节的问题， 老的gateway推送的时候会传输多一个字节
                        if (message->ParseFromArray(data, dataLen - 1))
                        {
                            result = message_ptr(message);
                        }
                        else
                        {
                            delete message;
                        }
                    }
                    else
                    {
                        delete message;
                    }
                }
            }
            else
            {
                result = message_ptr(message);
            }
        }
    }
    return result;
}

void SendData(const std::string& topic, const std::string& data)
{
    if (!m_producer)
    {
        cout << "create  producer" << endl;
        // 配置生产者
        std::string err_string;
        RdKafka::Conf* conf= RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        conf->set("bootstrap.servers", MapConfigFile["servers"].c_str(),err_string);
        if(!MapConfigFile["username"].empty() && !MapConfigFile["password"].empty())
        {
            cout << "username:" << MapConfigFile["username"] << " password:" << MapConfigFile["password"] << endl;
            conf->set("security.protocol", "SASL_PLAINTEXT",err_string);
            conf->set("sasl.mechanisms", "PLAIN",err_string);
            conf->set("sasl.username", MapConfigFile["username"].c_str(), err_string);
            conf->set("sasl.password", MapConfigFile["password"].c_str(), err_string);
        }

        m_producer = RdKafka::Producer::create(conf, err_string);
        delete conf;
        if (!m_producer) 
        {
            cout << "Failed to create producer: " << err_string << endl;
            return ;
        }
    }
    //cout << "SendData  topic:" << topic << endl;

    RdKafka::ErrorCode res = RdKafka::ERR_NO_ERROR;
    res = m_producer->produce(topic.c_str(), RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
        const_cast<char*>(data.data()), data.size(),
        nullptr,  0, 0, nullptr, nullptr);

    if (res != RdKafka::ErrorCode::ERR_NO_ERROR) 
    {
        cout << "Failed to produce message: " << RdKafka::err2str(res) << endl;
    } 
    else 
    {
        cout << "Message sent successfully!" << endl;
    }

    // 确保所有消息已发送
    m_producer->flush(10000);

    cout << "Message sent end!" << endl;
}

void protobuf_send(std::string strID, string strType, const google::protobuf::Message& message)
{
    std::string rsp_message;
    if (!message.SerializeToString(&rsp_message))
    {
        cout << "protobuf_send SerializeToString error!"<< endl;
        return ;
    }
    //数据发送协议
    subcenter::PluginDataRsp rsp;
    rsp.set_id(strID);
    rsp.set_type(strType);
    rsp.set_data(rsp_message);

    //序列化
    std::string strbody = protobuf_encode(rsp);
    ant::rpc::KafkaPayload kmsgSend;
    if (!strbody.empty())
    {
        kmsgSend.set_serialized_data(strbody);
        char* profile = NULL;
        profile = getenv("PROFILE");
        if (profile)
        {
            kmsgSend.set_profile(profile);
        }
        std::string rsp_str;
        if (kmsgSend.SerializeToString(&rsp_str))
        {
            SendData("upb_cosmos_plugin_rsp", rsp_str);
        }
    }
}

void PushData()
{
    if (m_set_push_topic.empty())
        return ;

    cout << "PushData"<< endl;
    for (const auto& itor : m_set_push_topic)
    {
        //这个是用户接收到的数据
        subcenter::SubcenterPush push;
        push.set_topic(itor);
        push.set_userid("654321");
        push.set_msg("这是一条测试消息");

        //这个是统一发送格式
        subcenter::SubcenterPush push2;
        push2.set_topic(itor);
        push2.set_userid("654321");
        push2.set_msg(protobuf_encode(push));
        protobuf_send("","SubcenterPush", push2);
    }
}

void message_callback(RdKafka::Message* msg) 
{
    const string& topicname = msg->topic_name();
    cout << "Message received topic_name:" << topicname << endl;

    ant::rpc::KafkaPayload kmsg;
    if (!(kmsg.ParseFromArray(msg->payload(),(msg->len())) && !kmsg.serialized_data().empty())) 
    {
        cout << "rpc::KafkaPayload fail" << endl;
    }

    std::string strData = kmsg.serialized_data();
    auto pb_req = protobuf_decode(strData);
    auto pluginData = dynamic_cast<const subcenter::PluginDataReq*>(pb_req.get());
    if(pluginData == NULL)
    {
        cout << "pluginData == NULL" << endl;
        return ;
    }
    std::string  strID = pluginData->id();
    if(strID.empty())
    {
        cout << "strID.empty()" << endl;
        return ;
    }

    std::string strType = pluginData->type();
    cout << "pluginData type: " << strType << endl;
    if (strType == "PluginQueryReq")
    {//查询请求
        auto reqData = std::make_shared<subcenter::PluginQueryReq>();
        if(!reqData->ParseFromString(pluginData->data()))
        {
            cout << "PluginQueryReq reqData == NULL" << endl;
            return ;
        }
        cout << "Received message user: " << reqData->user() <<" action:"<<reqData->action() << endl;
        subcenter::PluginQueryRsp rsp;
        rsp.set_code(0);
        rsp.set_msg("success");
        rsp.set_data("123456");
        protobuf_send(strID,"PluginDataRsp",rsp);
    }
    else if (strType == "SubscribeReq")
    {
        auto reqData = std::make_shared<subcenter::SubscribeReq>();
        if(!reqData->ParseFromString(pluginData->data()))
        {
            cout << "SubscribeReq reqData == NULL" << endl;
            return ;
        }

        std::string strTopic = reqData->topic();
        if(strTopic.empty())
        {
            cout << "strTopic.empty()" << endl;
            return ;
        }

        m_set_push_topic.insert(strTopic);
        cout << "message topic:"<< strTopic<< endl;

        //需要处理订阅数据请求
        subcenter::SubscribeRsp sub_rsp;
        sub_rsp.set_topic(strTopic);
        protobuf_send(strID,"SubscribeRsp",sub_rsp);
    }
    else if (strType == "UnSubscribeReq")
    {
        auto reqData = std::make_shared<subcenter::UnSubscribeReq>();
        if(!reqData->ParseFromString(pluginData->data()))
        {
            cout << "UnSubscribeReq reqData == NULL" << endl;
            return ;
        }

        std::string strTopic = reqData->topic();
        if(strTopic.empty())
        {
            cout << "strTopic.empty()" << endl;
            return ;
        }

        m_set_push_topic.erase(strTopic);
        cout << "message topic:"<< strTopic<< endl;

        //需要处理订阅数据请求
        subcenter::UnSubscribeRsp sub_rsp;
        sub_rsp.set_topic(strTopic);
        protobuf_send(strID,"UnSubscribeRsp",sub_rsp);
    }
    else if(strType == "test-topic")
    {
        cout << "Received message test-topic" <<endl;
    }
}

int main() 
{
    //读取配置
    ReadConfigFile();

    // 配置消费者
    std::string err_string;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", MapConfigFile["servers"].c_str(), err_string);
    conf->set("group.id", MapConfigFile["groupid"].c_str()/*"console-consumer-10892"*/,err_string);
    conf->set("auto.offset.reset", "earliest",err_string);

    if(!MapConfigFile["username"].empty() && !MapConfigFile["password"].empty())
    {
        cout << "username:" << MapConfigFile["username"] << " password:" << MapConfigFile["password"] << endl;
        conf->set("security.protocol", "SASL_PLAINTEXT",err_string);
        conf->set("sasl.mechanisms", "PLAIN",err_string);
        conf->set("sasl.username", MapConfigFile["username"].c_str(), err_string);
        conf->set("sasl.password", MapConfigFile["password"].c_str(), err_string);
    }

    RdKafka::KafkaConsumer* consumer = RdKafka::KafkaConsumer::create(conf, err_string);
    delete conf;

    std::string strTopics = MapConfigFile["sub_topics"];
    // 确保消费者订阅正确的主题
    std::vector<std::string> topics = splitString(strTopics);
    //std::vector<std::string> topics = {"upb_cosmos_plugin_req"};

    RdKafka::ErrorCode code = consumer->subscribe(topics);
    if (code)
    {
        cout << "Failed to subscribe: " << code <<endl;
        return 1;
    }

    cout << "Waiting for messages..." << endl;

    while (true) 
    {
        if(++m_sub_push_count % 10 == 0)
        {
            //进行推送代码实现
            PushData();
        }

        RdKafka::Message* msg = consumer->consume(1000);
        if (msg->err() == RdKafka::ERR_NO_ERROR) 
        {
            message_callback(msg);
        }
        else if (msg->err() == RdKafka::ERR__TIMED_OUT) 
        {
            //cout << "ERR__TIMED_OUT" << endl;
            continue; // 继续等待
        } 
        else 
        {
            cerr << "Consumer error: " << msg->errstr() << endl;
            break;
        }
        delete msg;
        consumer->commitAsync();
    }

    cout << "Waiting return" << endl;
    return 0;
}
