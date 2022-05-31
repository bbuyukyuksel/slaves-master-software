#ifndef MY_MQTT_PUBLISHER
#define MY_MQTT_PUBLISHER

#include <iostream>
#include <mosquitto.h>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>


class MosquittoPublisher {
	mosquitto* m_mosq;
	std::string m_id;
	std::string m_server;
	size_t m_port;
	size_t m_keep_alive_secs;
	size_t m_mqtt_restart_time_sec;
	time_t tick_time;
	
public:
	MosquittoPublisher() = delete;
	MosquittoPublisher(const char * id, const char * server = "localhost", size_t port = 1883, size_t keep_alive_secs = 60, size_t mqtt_restart_time_sec=5) :
		m_mosq(mosquitto_new(id, true, NULL)),
		m_id(id), m_server(server), m_port(port), m_keep_alive_secs(keep_alive_secs), m_mqtt_restart_time_sec(mqtt_restart_time_sec)
	{
		mosquitto_lib_init();
		Connect();
	}

	void Connect() {
		int try_counter{};
		int rc;
		do{
			if (try_counter > 10) exit(EXIT_FAILURE);

			rc = mosquitto_connect(m_mosq, m_server.c_str(), m_port, m_keep_alive_secs);
			printf("Client could not connect to broker! Error Code: %d Try: %d\n", rc, ++try_counter);
			if (rc != 0){
				mosquitto_destroy(m_mosq);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		} while (rc != 0);

		time(&tick_time);
		printf("Connected to the broker!\n");
	}

	void Reconnect() {
		time_t tock_time;
		time(&tock_time);

		auto diff = difftime(tock_time, tick_time);

		if (diff > m_mqtt_restart_time_sec) {
			time(&tick_time);
			std::cout << "Reconnecting to MQTT Broker..\n#Response:";
			
			int status_code = mosquitto_reconnect(m_mosq);
			switch (status_code)
			{
			case MOSQ_ERR_SUCCESS:
				std::cout << "MOSQ_ERR_SUCCESS\n"; break;
			case MOSQ_ERR_INVAL:
				std::cout << "MOSQ_ERR_INVAL\n"; break;
			case MOSQ_ERR_NOMEM:
				std::cout << "MOSQ_ERR_NOMEM\n"; break;
			case MOSQ_ERR_ERRNO:
				std::cout << "MOSQ_ERR_ERRNO\n"; break;
			default:
				std::cout << "Something gone wrong!\n";
				break;
			}
		}
		else {
			std::cout << "will restart " << m_mqtt_restart_time_sec - diff << "\n";
		}

		
	}

	void Publish(std::string topic, std::string payload)
	{
		std::cout << "[Publish] Topic: " << topic << ", Payload: " << payload << "\n";
		int status_code = mosquitto_publish(m_mosq, NULL, topic.c_str(), payload.length(), payload.c_str(), 0, false);
		/*switch (status_code)
		{
		case MOSQ_ERR_SUCCESS:
			std::cout << "MOSQ_ERR_SUCCESS\n";
			break;
		case MOSQ_ERR_INVAL:
			std::cout << "MOSQ_ERR_INVAL\n";
			break;
		case MOSQ_ERR_NOMEM:
			std::cout << "MOSQ_ERR_NOMEM\n";
			break;
		case MOSQ_ERR_NO_CONN:
			std::cout << "MOSQ_ERR_NO_CONN\n";
			break;
		case MOSQ_ERR_PROTOCOL:
			std::cout << "MOSQ_ERR_PROTOCOL\n";
			break;
		case MOSQ_ERR_PAYLOAD_SIZE:break;
			std::cout << "MOSQ_ERR_PAYLOAD_SIZE\n";
			break;
		case MOSQ_ERR_MALFORMED_UTF8:break;
			std::cout << "MOSQ_ERR_MALFORMED_UTF8\n";
			break;
		case MOSQ_ERR_QOS_NOT_SUPPORTED:break;
			std::cout << "MOSQ_ERR_QOS_NOT_SUPPORTED\n";
			break;
		case MOSQ_ERR_OVERSIZE_PACKET:break;
			std::cout << "MOSQ_ERR_OVERSIZE_PACKET\n";
			break;
		default:
			std::cout << "Something gone wrong!\n";
			break;
		}*/
	}

	void Close() {
		mosquitto_disconnect(m_mosq);
		free(m_mosq);
		m_mosq = nullptr;
		mosquitto_lib_cleanup();
	}

	void Info()
	{
		std::cout << "ID:" << m_id << std::endl;
		std::cout << "Server:" << m_server << std::endl;
		std::cout << "Port:" << m_port << std::endl;
		std::cout << "Keep Alive (secs):" << m_keep_alive_secs << std::endl;
	}

	static void TestCode() {
		auto mypublisher = MosquittoPublisher("test-user");
		mypublisher.Publish("test/t1", "Hello World!");
		mypublisher.Info();
	}
};
#endif