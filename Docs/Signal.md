# Signal

`Signal` - это реализация паттерна Signal/Slot (сигналы и слоты) на C++, предоставляющая механизм публикации-подписки (publish-subscribe) для событийно-ориентированного программирования.

**Заголовочный файл:** `Awl/Signal.h`

## Обзор

Класс `Signal<Args...>` позволяет объектам (издателям) рассылать уведомления подписчикам (слотам) без жесткой связанности между ними. Это основа для реализации слабо связанных систем, где компоненты взаимодействуют через события.

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
- `Slot` - тип функции-обработчика (слота), которая может быть сравнена на равенство

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

// Подписка с weak_ptr (автоматическая отписка при уничтожении)
template <class Object>
void subscribe(std::weak_ptr<Object> p_object, void (Object::*member)(Args...));
```

### Отписка

```cpp
// Отписка по ID
bool unsubscribe(Id id);

// Отписка сырым указателем
template <class Object>
bool unsubscribe(Object* p_object, void (Object::*member)(Args...));

// Отписка по weak_ptr/shared_ptr
template <class Object>
bool unsubscribe(std::shared_ptr<Object> p_object, void (Object::*member)(Args...));
```

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

## Особенности реализации

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

## Зависимости

- `Awl/EquatableFunction.h` - типобезеричная функция с поддержкой сравнения
- `Awl/UniqueId.h` - генерация уникальных идентификаторов
- `<concepts>` - C++20 concepts для ограничений шаблонов
- `<functional>` - std::function
- `<memory>` - умные указатели
- `<vector>` - контейнер для хранения слотов

## Требования компилятора

- C++20 или выше (используются concepts)
- Поддержка `std::bit_cast` (C++23) или альтернативы
