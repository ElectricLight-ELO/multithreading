# Выполнение тестового задания на многопоточность
## Шаги

### Настройка среды
Работаю обычно в VS / VS code, для разработки на linux использую VS code. Для работы в связке VS Code -> WSL -> Docker использую расширение  `Dev Containers`.

Разграничил Dockerfile и compose.yaml. Отладка работает. Зависимости такие- cmake 4.4.0, g++ 14.2.0, Ninja 1.13.2, Vcpkg(установлен)

### Идея выполнения
Задание решил выполнять на базе общей и локальной очередей. Под сообщения создал структуру `Message` с фиксированным array на 256 символов и фактической длиной текста. Так как string_view сам по себе не владеет данными, входной текст копируем в буфер сообщения.

Общая очередь используется для приёма сообщений от нескольких потоков-производителей, а локальная служит для их пакетной обработки единственным потоком-потребителем.

Потребитель под mutex обменивает общую очередь с локальной, затем освобождает блокировку и печатает сообщения. Это позволяет не удерживать mutex во время печати и не мешать производителям добавлять новые сообщения.


### Проблемы с которыми столкнулся во время выполнения и как выходил
1. **Проблемы по синхронизации**: Случайно в `post` захватывал один и тот же mutex по невнимательности, начал запускать первично - заметил проблему. Исправил границы блокировки: подготовку сообщения и печать вынес за пределы критической секции, под mutex оставил только операции с общей очередью.
2. Чтобы поток-потребитель при пустой очереди не крутился в цикле с постоянными проверками. Для этого использовал condition_variable_any. Потребитель ожидает пока нет сообщений, после добавления сообщения в `post` вызывается notify_one.

### Benchmarks
Release-
<img width="650" height="230" alt="тест release" src="https://github.com/user-attachments/assets/2878c6fe-7939-457c-8a48-5d0e58bd48bd" />

Debug-
<img width="650" height="230" alt="тест debug" src="https://github.com/user-attachments/assets/c0d65a7e-3818-498f-8a7a-f337eb9e71c5" />

----
### Комманды отлажены:
```sh
# Install docker-compose(1).
sudo apt install -y docker-compose

# Build image.
docker compose build

# Configure project.
docker compose run --rm -u "$(id -u):$(id -g)" dev cmake --preset default

# Build and run main.
docker compose run --rm -u "$(id -u):$(id -g)" dev cmake --build --preset debug
docker compose run --rm -u "$(id -u):$(id -g)" dev build/Debug/main

# Build and run tests.
docker compose run --rm -u "$(id -u):$(id -g)" dev cmake --build --preset debug --target tests
docker compose run --rm -u "$(id -u):$(id -g)" dev build/src/tests/Debug/tests

# Build and run benchmarks.
docker compose run --rm -u "$(id -u):$(id -g)" dev cmake --build --preset release --target benchmarks
docker compose run --rm -u "$(id -u):$(id -g)" dev setarch --addr-no-randomize build/src/benchmarks/Release/benchmarks
```
