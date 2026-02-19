# Signal

`Signal` - это реализация паттерна Signal/Slot (сигналы и слоты) на C++, предоставляющая механизм публикации-подписки (publish-subscribe) для событийно-ориентированного программирования.

**Заголовочный файл:** `Awl/Signal.h`

## Обзор

Класс `Signal<Args...>` позволяет объектам (издателям) рассылать уведомления подписчикам (слотам) без жесткой связанности между ними. Это основа для реализации слабо связанных систем, где компоненты взаимодействуют через события.

### Архитектура

```
┌─────────────────────┐      std::vector      ┌──────────────────────┐
│   Signal<Args...>   │◄─────────────────────│ equatable_function   │
│                     │                      │  (type-erased call)  │
│ - m_slots           │                      └──────────────────────┘
└─────────────────────┘
```

**Ключевые особенности:**
- Использует `std::vector` для хранения слотов
- Поддержка `equatable_function` с type erasure
- Автоматическая очистка недействительных `weak_ptr`
- Поддержка умных указателей (`shared_ptr`/`weak_ptr`)

### Типы

```cpp
template <class... Args>
class Signal
{
public:
    using Slot = equatable_function<void(Args...)>;
    using container_type = std::vector<Slot>;
};
```

- `Args...` - типы аргументов, передаваемых при эмите сигнала
- `Slot` - тип функции-обработчика (слота) с поддержкой сравнения на равенство
- `Id` - уникальный идентификатор для отписки (определяется в `Awl/UniqueId.h`)

## Основные возможности

| Возможность | Описание |
|-------------|----------|
| **Подписка с ID** | Возвращает уникальный идентификатор для последующей отписки |
| **Raw pointers** | Подписка через сырой указатель на объект |
| **std::shared_ptr** | Подписка с продлением времени жизни объекта |
| **std::weak_ptr** | Подписка без продления времени жизни, автоматическая отписка при уничтожении |
| **Лямбда-выражения** | Подписка с использованием лямбд и std::function |
| **Автоматическая очистка** | Недействительные weak_ptr автоматически удаляются при эмите |
| **Const/Constless** | Поддержка как const, так и non-const методов-членов |

## Основные методы

### Подписка

```cpp
// Подписка с использованием std::function, возвращает ID
Id subscribe(std::function<void(Args...)> func);

// Подписка сырым указателем
template <class Object>
void subscribe(Object* p_object, void (Object::*member)(Args...));

// Подписка сырым указателем на const-метод
template <class Object>
void subscribe(const Object* p_object, void (Object::*member)(Args...) const);

// Подписка с shared_ptr (продлевает время жизни объекта)
template <class Object>
void subscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...));

// Подписка с shared_ptr на const-метод
template <class Object>
void subscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...) const);

// Подписка с weak_ptr (автоматическая отписка при уничтожении)
template <class Object>
void subscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...));

// Подписка с weak_ptr на const-метод
template <class Object>
void subscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...) const);
```

**Примечание:** const-методы позволяют подписываться на методы, которые не изменяют состояние объекта. Это полезно для immutable объектов или для соблюдения const-корректности.

### Отписка

```cpp
// Отписка по ID (возвращает true если подписчик был найден и удален)
bool unsubscribe(Id id);

// Отписка сырым указателем
template <class Object>
bool unsubscribe(Object* p_object, void (Object::*member)(Args...));

// Отписка const-метода
template <class Object>
bool unsubscribe(const Object* p_object, void (Object::*member)(Args...) const);

// Отписка по shared_ptr
template <class Object>
bool unsubscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...));

// Отписка по shared_ptr const-метода
template <class Object>
bool unsubscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...) const);

// Отписка по weak_ptr
template <class Object>
bool unsubscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...));

// Отписка по weak_ptr const-метода
template <class Object>
bool unsubscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...) const);
```

**Возвращаемое значение:** `true` если подписчик был найден и удален, `false` если подписчик не найден.

### Эмит

```cpp
// Отправка сигнала всем подписчикам
template<typename ...Params>
void emit(const Params&... args) const;
```

### Утилиты

```cpp
// Очистка всех подписчиков
void clear() noexcept;

// Проверка на отсутствие подписчиков
bool empty() const noexcept;

// Количество подписчиков
std::size_t size() const noexcept;
```

## Use Cases

### Use Case 1: Базовая подписка с сырым указателем

```cpp
#include "Awl/Signal.h"
#include <iostream>

class Button
{
public:
    awl::Signal<> onClick;
};

class Dialog
{
public:
    void onClose()
    {
        std::cout << "Dialog closing\n";
    }

    void onButtonClick()
    {
        std::cout << "Button clicked!\n";
    }
};

int main()
{
    Button button;
    Dialog dialog;

    // Подписываем метод Dialog на клик кнопки
    button.subscribe(&dialog, &Dialog::onButtonClick);

    // Эмитим сигнал
    button.emit();  // Вывод: Button clicked!

    return 0;
}
```

### Use Case 2: Подписка с аргументами

```cpp
class TemperatureSensor
{
public:
    // Сигнал передает значение температуры
    awl::Signal<double> onTemperatureChange;
};

class Thermostat
{
public:
    void updateTemperature(double temp)
    {
        std::cout << "Temperature: " << temp << " C\n";
        if (temp > 25.0)
        {
            std::cout << "Cooling...\n";
        }
    }
};

int main()
{
    TemperatureSensor sensor;
    Thermostat thermostat;

    sensor.subscribe(&thermostat, &Thermostat::updateTemperature);

    sensor.emit(23.5);  // Temperature: 23.5 C
    sensor.emit(27.0);  // Temperature: 27.0 C, Cooling...

    return 0;
}
```

### Use Case 3: Лямбда-выражения с отпиской по ID

```cpp
class Counter
{
public:
    awl::Signal<int> onCount;
};

int main()
{
    Counter counter;

    // Подписываем лямбду и получаем ID
    awl::Id id = counter.subscribe([](int value) {
        std::cout << "Count: " << value << "\n";
    });

    counter.emit(1);  // Count: 1
    counter.emit(2);  // Count: 2

    // Отписываемся по ID
    counter.unsubscribe(id);

    counter.emit(3);  // Ничего не выводится

    return 0;
}
```

### Use Case 4: weak_ptr для автоматической отписки

```cpp
#include <memory>

class NewsPublisher
{
public:
    awl::Signal<const std::string&> publishNews;
};

class NewsSubscriber : public std::enable_shared_from_this<NewsSubscriber>
{
public:
    NewsSubscriber(const std::string& name) : m_name(name) {}

    void receive(const std::string& news)
    {
        std::cout << m_name << " received: " << news << "\n";
    }

private:
    std::string m_name;
};

int main()
{
    auto pub = std::make_shared<NewsPublisher>();

    {
        auto sub1 = std::make_shared<NewsSubscriber>("Alice");
        auto sub2 = std::make_shared<NewsSubscriber>("Bob");

        // Подписываемся через weak_ptr - объект не будет удерживаться
        pub->subscribe(std::weak_ptr<NewsSubscriber>(sub1), &NewsSubscriber::receive);
        pub->subscribe(std::weak_ptr<NewsSubscriber>(sub2), &NewsSubscriber::receive);

        pub->publishNews.emit("Breaking news!");
        // Alice received: Breaking news!
        // Bob received: Breaking news!

        // sub1 и sub2 уничтожаются здесь
    }

    std::cout << "After scope:\n";
    pub->publishNews.emit("Another news");
    // Ничего не выводится - все подписчики автоматически отписаны

    return 0;
}
```

### Use Case 5: shared_ptr для продления жизни

```cpp
class Timer
{
public:
    awl::Signal<> onTimeout;
};

class TimerHandler : public std::enable_shared_from_this<TimerHandler>
{
public:
    TimerHandler() { std::cout << "Handler created\n"; }
    ~TimerHandler() { std::cout << "Handler destroyed\n"; }

    void handle()
    {
        std::cout << "Timer fired!\n";
    }
};

int main()
{
    Timer timer;
    {
        auto handler = std::make_shared<TimerHandler>();

        // shared_ptr продлевает жизнь handler до отписки
        timer.subscribe(handler, &TimerHandler::handle);

        // handler может быть уничтожен здесь, но подписка удерживает объект
    }

    std::cout << "After scope:\n";
    timer.onTimeout.emit();  // Timer fired!

    timer.onTimeout.clear();  // Handler destroyed

    return 0;
}
```

### Use Case 6: Несколько аргументов

```cpp
class CoordinateSystem
{
public:
    awl::Signal<double, double, double> onPositionChanged;
};

class Robot
{
public:
    void moveTo(double x, double y, double z)
    {
        std::cout << "Moving to (" << x << ", " << y << ", " << z << ")\n";
    }
};

int main()
{
    CoordinateSystem cs;
    Robot robot;

    cs.subscribe(&robot, &Robot::moveTo);

    cs.onPositionChanged.emit(1.0, 2.0, 3.0);
    // Moving to (1, 2, 3)

    return 0;
}
```

### Use Case 7: Const-методы

```cpp
class DataLogger
{
public:
    void logData(int value) const
    {
        std::cout << "Logging: " << value << "\n";
    }
};

class DataSource
{
public:
    awl::Signal<int> onData;
};

int main()
{
    DataSource source;
    const DataLogger logger;  // const объект

    // Подписка на const-метод
    source.subscribe(&logger, &DataLogger::logData);

    source.onData.emit(42);  // Logging: 42

    return 0;
}
```

### Use Case 8: Проверка дубликатов

```cpp
Signal<int> sig;
auto handler = [](int x) { std::cout << x << "\n"; };

// Первая подписка успешна
awl::Id id1 = sig.subscribe(handler);

// Попытка подписать тот же обработчик снова
// - будет проигнорирована из-за проверки на дубликаты
awl::Id id2 = sig.subscribe(handler);

std::cout << "Subscribers: " << sig.size() << "\n";  // Вывод: 1
```

## Особенности реализации

### equatable_function

`equatable_function` — это type-erased функция с поддержкой сравнения на равенство:

```cpp
using Slot = equatable_function<void(Args...)>;
```

**Преимущества:**
- Позволяет хранить любой callable объект (лямбды, функции, методы)
- Поддерживает сравнение для корректной работы `unsubscribe`
- Работает с умными указателями

**Как работает сравнение:**
```cpp
// Два слота равны если они указывают на:
// 1. Один и тот же объект и один и тот же метод
// 2. Один и тот же ID (для std::function)
Slot slot1(&obj, &Class::method);
Slot slot2(&obj, &Class::method);
// slot1 == slot2 -> true
```

### Проверка дубликатов

При подписке `Signal` проверяет, существует ли уже такой слот:

```cpp
void subscribe(Slot slot)
{
    if (std::find(m_slots.begin(), m_slots.end(), slot) == m_slots.end())
    {
        m_slots.push_back(std::move(slot));
    }
}
```

Это предотвращает дублирование подписок на один и тот же метод объекта.

### Swap-and-pop стратегия

При удалении слотов используется swap-and-pop для O(1) удаления:

```cpp
bool unsubscribe(const Slot& slot)
{
    const auto it = std::find(m_slots.begin(), m_slots.end(), slot);

    if (it == m_slots.end())
    {
        return false;
    }

    auto last = m_slots.end();
    --last;

    if (it != last)
    {
        *it = std::move(*last);  // Перемещаем последний элемент
    }

    m_slots.pop_back();  // Удаляем последний элемент
    return true;
}
```

**Примечание:** Это изменяет порядок слотов, но не влияет на функциональность.

### Автоматическая очистка мертвых подписчиков

При использовании `weak_ptr` и вызове `emit()`, сигнал автоматически проверяет валидность слабых ссылок. Если объект уничтожен, соответствующий слот удаляется из списка подписчиков.

```cpp
while (i != m_slots.end())
{
    auto guard = i->lock();  // Пытаемся захватить weak_ptr

    if (guard)
    {
        guard(args...);  // Вызываем слот
        ++i;
    }
    else
    {
        // Удаляем мертвый слот
        *i = std::move(*last);
        m_slots.pop_back();
    }
}
```

### Сравнимость слотов

`equatable_function` позволяет сравнивать слоты на равенство, что обеспечивает корректную работу `unsubscribe`. Два слота считаются равными, если они указывают на один и тот же объект и один и тот же метод-член.

## Compile-time проверки

`Signal` использует C++20 concepts для проверки вызываемости слотов:

```cpp
template<typename ...Params>
void emit(const Params&... args) const
    requires (std::invocable<Slot&, const Params&...>)
```

Это обеспечивает compile-time проверку типов аргументов:

```cpp
Signal<int, std::string> sig;

sig.emit(42, "hello");      // OK
sig.emit("wrong", 42);      // Ошибка компиляции: несовпадение типов
sig.emit(42);               // Ошибка компиляции: недостаточно аргументов
```

## Thread Safety

`Signal` **не является потокобезопасным**. Используйте внешнюю синхронизацию при работе с несколькими потоками.

### Многопоточные сценарии

1. **Подписка/отписка из других потоков:** Требуется синхронизация
2. **Эмит из других потоков:** Требуется синхронизация
3. **Автоматическая очистка weak_ptr:** Происходит в потоке, вызывающем `emit()`

## Best Practices

### 1. Используйте weak_ptr для предотвращения циклических зависимостей

```cpp
// Хорошо: weak_ptr не продлевает жизнь объекта
class Subscriber : public std::enable_shared_from_this<Subscriber>
{
public:
    void subscribe(Signal<int>& sig)
    {
        sig.subscribe(std::weak_ptr<Subscriber>(shared_from_this()),
                      &Subscriber::onEvent);
    }
};

// Плохо: shared_ptr может создать циклическую зависимость
class Subscriber
{
public:
    void subscribe(Signal<int>& sig)
    {
        sig.subscribe(shared_from_this(), &Subscriber::onEvent);
        // Signal удерживает Subscriber, Subscriber удерживает Signal - цикл!
    }
};
```

### 2. Храните ID для отписки лямбд

```cpp
class MyClass
{
    awl::Signal<int> sig;
    awl::Id lambdaId;

public:
    void setup()
    {
        lambdaId = sig.subscribe([this](int value) {
            handle(value);
        });
    }

    void cleanup()
    {
        sig.unsubscribe(lambdaId);  // Отписываем лямбду по ID
    }
};
```

### 3. Проверяйте результат отписки

```cpp
bool removed = sig.unsubscribe(id);
if (!removed)
{
    std::cout << "Подписчик не найден (уже отписан или не существовал)\n";
}
```

### 4. Используйте const-методы для неизменяемых обработчиков

```cpp
class ImmutableHandler
{
public:
    void process(int value) const  // const-метод
    {
        // Только чтение, не изменяет состояние
    }
};

const ImmutableHandler handler;
sig.subscribe(&handler, &ImmutableHandler::process);
```

### 5. Очищайте сигналы перед уничтожением

```cpp
class Component
{
    awl::Signal<> onDestroy;

public:
    ~Component()
    {
        onDestroy.clear();  // Очищаем все подписки
        // Это предотвращает вызовы уже уничтоженного объекта
    }
};
```

## Требования компилятора

- C++20 или выше (используются concepts)
- Поддержка `std::invocable` concept
- Поддержка `std::convertible_to` concept
