#include "../../include/server/kafka.hpp"
#include <cstdlib>
#include <unistd.h>

#include <iostream>
#include<algorithm>
#include <librdkafka/rdkafkacpp.h>
using namespace std;



Kafka::Kafka()
  : _producer(nullptr), _producer_conf(nullptr),
    _consumer(nullptr), _consumer_conf(nullptr),
    _running(false) {}

Kafka::~Kafka() {
    unsubscribe();

    if (_producer) {
        _producer->flush(10000);
        delete _producer;
    }
    if (_producer_conf) delete _producer_conf;
    if (_consumer) delete _consumer;
    if (_consumer_conf) delete _consumer_conf;
}

bool Kafka::connect(const std::string& brokers) {
    std::string errstr;
    // —— 生产者配置 —— 
    _producer_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    if (_producer_conf->set("bootstrap.servers", brokers, errstr)
        != RdKafka::Conf::CONF_OK) {
        std::cerr << "Producer config error: " << errstr << std::endl;
        return false;
    }
    _producer = RdKafka::Producer::create(_producer_conf, errstr);
    if (!_producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        return false;
    }

    // —— 消费者配置 —— 
     _consumer_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    _consumer_conf->set("bootstrap.servers", brokers, errstr);

    // 给每个实例一个唯一组 ID，例如用进程 ID 或 UUID
    std::string unique_gid = "chat_service_group_" + std::to_string(getpid());
    _consumer_conf->set("group.id", unique_gid, errstr);

    // 最早 offset
    _consumer_conf->set("auto.offset.reset", "earliest", errstr);
    _consumer_conf->set("enable.auto.commit", "true", errstr);
    return true;
}

bool Kafka::publish(const std::string& topicName,
                    const std::string& message) {
    if (!_producer) {
        std::cerr << "Producer 未初始化\n";
        return false;
    }

    std::string errstr;
    // 1) 创建一个 RdKafka::Topic* 对象
    RdKafka::Topic* topic = RdKafka::Topic::create(
        _producer,         // 对应的 Producer 对象
        topicName,         // 主题名称
        nullptr,           // Topic 配置，这里用默认
        errstr             // 错误描述输出
    );
    if (!topic) {
        std::cerr << "创建 Topic 对象失败: " << errstr << std::endl;
        return false;
    }

    // 2) 调用 produce()，注意最后两个参数是 key 指针和 key 长度
    RdKafka::ErrorCode resp = _producer->produce(
        topic,                    // 第一个参数必须是 Topic*
        RdKafka::Topic::PARTITION_UA,        // 自动分区
        RdKafka::Producer::RK_MSG_COPY,      // 复制消息内容
        const_cast<char*>(message.data()),   // payload 指针
        message.size(),                      // payload 长度
        /* key_ptr */ nullptr,               // key 指针（可选）
        /* key_len */ 0,                     // key 长度
        /* msg_opaque */ nullptr             // 自定义透传指针
    );
    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "发送消息失败: " << RdKafka::err2str(resp) << std::endl;
        delete topic;
        return false;
    }

    // 3) 轻量轮询一次，触发 delivery report 回调
    _producer->poll(0);

    // 4) flush 最多等待2秒，确保所有消息发送到 broker
    resp = _producer->flush(2000);
    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "flush 失败: " << RdKafka::err2str(resp)
                  << "（可能有消息丢失）" << std::endl;
        delete topic;
        return false;
    }

    delete topic;
    return true;
}



bool Kafka::subscribe(const std::string& topic) {
    std::string errstr;
    // 首次调用时创建 consumer
    if (!_consumer) {
        _consumer = RdKafka::KafkaConsumer::create(_consumer_conf, errstr);
        if (!_consumer) {
            std::cerr << "Failed to create consumer: " << errstr << std::endl;
            return false;
        }
    }
    // 去重加入列表
    if (std::find(_subscribedTopics.begin(), _subscribedTopics.end(), topic)
        == _subscribedTopics.end()) {
        _subscribedTopics.push_back(topic);
    }
    // 重新订阅所有主题
    RdKafka::ErrorCode code = _consumer->subscribe(_subscribedTopics);
    if (code != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Subscribe error: " << RdKafka::err2str(code) << std::endl;
        return false;
    }
    // 启动消费线程（只启动一次）
    if (!_running) {
        _running = true;
        _consumer_thread = std::thread(&Kafka::consume_loop, this);
    }
    return true;
}

void Kafka::unsubscribe() {
    _running = false;
    if (_consumer) {
        _consumer->unsubscribe();
    }
    if (_consumer_thread.joinable()) {
        _consumer_thread.join();
    }
    _subscribedTopics.clear();
}

void Kafka::init_notify_handler(NotifyHandler handler) {
    _notify_handler = std::move(handler);
}

void Kafka::consume_loop() {
    std::string group_id;
    std::string errstr;
    _consumer_conf->get("group.id", group_id);
    std::cout << "[Kafka] consume_loop started for group=" << group_id << std::endl;
    while (_running) {
        RdKafka::Message* msg = _consumer->consume(100);
        if (!msg) continue;
        if (msg->err() == RdKafka::ERR_NO_ERROR) {
            std::string topic = msg->topic_name();
            std::string payload(static_cast<char*>(msg->payload()), msg->len());
            if (_notify_handler) {
                _notify_handler(topic, payload);
            }
        } 
        delete msg;
    }
}
