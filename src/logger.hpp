#pragma once
#include <format>
#include <stop_token>
#include <queue>
#include <cstring>
#include <condition_variable>

// структура под сообщение
struct Message {
    std::array<char, 256> data;
    std::uint16_t size;
};



extern void (*print)(std::string_view message);


class logger {
  std::queue<Message> message_queue;
  // мьютекс именно для очереди
  std::mutex mutex_;
  std::condition_variable_any cv_;
  
public:
  /// Called on many threads. Must queue the message.
  /// Should return as soon as possible and rarely block.
  void post(std::string_view message) {
    std::lock_guard<std::mutex> lock(mutex_);

    
    Message msg;
    if(message.size() > msg.data.size()){
      throw std::runtime_error("Message too long");
   //   return; // сообщение слишком длинное, игнорируем
    }

    msg.size = static_cast<std::uint16_t>(message.size());  // создание копии
   if(!message.empty()) {
        std::memcpy(msg.data.data(), message.data(), message.size()); // Копирование в структуру Message
    } else {
        msg.size = 0;
    }

  {
    // блокаем мьютекс и пушим сообещние в очередь
    std::lock_guard<std::mutex> lock(mutex_);
    message_queue.push(msg);
   }
    

    cv_.notify_one();  // (сигнал) работа в очереди появилась
  }


  /// Called on many threads. Must format and queue the message.
  /// Should return as soon as possible and rarely block.
  template <class... Args>
  void post(std::format_string<Args...> fmt, Args&&... args) {
    post(std::format(fmt, std::forward<Args>(args)...));
  }

  /// Called on one thread. Must call `print` once per message.
  void run(std::stop_token stop) {
    std::queue<Message> local_queue;
    
    // цикл обработки сообщений

    std::unique_lock lock_th(mutex_);
    lock_th.lock();

    while (!message_queue.empty() && !stop.stop_requested()) {


    } 


   // std::string_view view{msg.data.data(), msg.size};
   // print(view);  
  }
};
