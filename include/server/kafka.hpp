#pragma once
#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <librdkafka/rdkafkacpp.h>

class Kafka {
public:
    using NotifyHandler = std::function<void(const std::string& topic, const std::string& message)>;

    Kafka();
    ~Kafka();

    // 连接到 brokers
    bool connect(const std::string& brokers);
    // 发布消息
    bool publish(const std::string& topic, const std::string& message);
    // 订阅一个新主题，允许多次调用
    bool subscribe(const std::string& topic);
    // 取消所有订阅并停止消费线程
    void unsubscribe();

    // 设置从 Kafka 收到消息的回调
    void init_notify_handler(NotifyHandler handler);

private:
    void consume_loop();

    RdKafka::Producer*     _producer;
    RdKafka::Conf*         _producer_conf;

    RdKafka::KafkaConsumer* _consumer;
    RdKafka::Conf*          _consumer_conf;

    std::vector<std::string> _subscribedTopics;
    std::thread              _consumer_thread;
    bool                     _running;

    NotifyHandler           _notify_handler;
};
