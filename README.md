这是一个集群聊天服务器/基于mudo网络库实现的ChatServer聊天服务器
该项目由muduo开源网络库，json序列化，数据库，kafka和nginx反向代理等技术点完成
1，基于开源的muduo网络库进行服务间的通信，提供高并发网络I/O服务解耦网络层和业务处理
2，利用json的序列化和反序列化完成通信的私有传输
3，利用MySQL关系型数据库完成项目的落地存储
4，利用kafka的发布订阅模式，实现跨服务器通信
5，使用nginx的TCP负载均衡实现服务器的集群功能，提高服务的并发能力
