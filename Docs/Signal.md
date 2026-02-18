# Signal (AWL)

## Что это

`awl::Signal<Args...>` — это сигнал со слотами на базе `equatable_function`.
Поддерживает функциональный стиль подписок: лямбды, методы объектов, `shared_ptr`, `weak_ptr`.

## Основные фичи

- Подписка:
  - `std::function<void(Args...)>`
  - `Object* + member`
  - `shared_ptr<Object> + member`
  - `weak_ptr<Object> + member`
- Отписка:
  - по `Id` (для `std::function`)
  - по object/member
  - по `Slot`
- Дедупликация одинаковых подписок.
- `emit(args...)` рассылает событие всем слотам.
- Автоочистка истекших `weak_ptr` слотов во время `emit`.
- `clear`, `empty`, `size`.

## Базовый use case: подписки на метод и лямбду

```cpp
#include "Awl/Signal.h"
#include <functional>

class Handler
{
public:
    void onPrice(int value)
    {
        sum += value;
    }

    int sum = 0;
};

void Example()
{
    awl::Signal<int> signal;

    Handler h;
    signal.subscribe(&h, &Handler::onPrice);

    int total = 0;
    auto id = signal.subscribe(std::function<void(int)>(
        [&total](int v) { total += v * 2; }
    ));

    signal.emit(10);
    // h.sum == 10, total == 20

    signal.unsubscribe(id);
    signal.unsubscribe(&h, &Handler::onPrice);
}
```

## Use case: weak_ptr без утечек подписок

```cpp
#include "Awl/Signal.h"
#include <memory>

class Session
{
public:
    void onTick(int) {}
};

void ExampleWeak()
{
    awl::Signal<int> tick;

    auto owner = std::make_shared<Session>();
    std::weak_ptr<Session> weak = owner;

    tick.subscribe(weak, &Session::onTick);

    tick.emit(1);  // slot будет вызван

    owner.reset(); // объект уничтожен
    tick.emit(2);  // expired slot будет удален автоматически
}
```

## Когда выбирать Signal

- Нужны гибкие callback-подписки (лямбды + методы объектов).
- Нужна интеграция со `shared_ptr/weak_ptr` владением.
- Удобно управлять подписками через `Id`.
- Нужен простой public API `emit` без отдельного интерфейса observer.

