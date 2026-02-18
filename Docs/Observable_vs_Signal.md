# Observable vs Signal (AWL)

## Коротко

- `Observable` — OOP-подход через интерфейс observer.
- `Signal` — функциональный подход через слоты/callback-и.

## Сравнение

| Критерий | Observable | Signal |
|---|---|---|
| Модель | `IObserver` + `Observer<I>` + `Observable<I>` | `Signal<Args...>` + `equatable_function` |
| Подписка | указатель на observer | lambda/std::function, object+member, shared/weak |
| Отписка | observer pointer или self-unsubscribe | по `Id`, по slot, по object/member |
| Дедупликация | обычно нет встроенной проверки дубля | есть проверка дублей |
| Short-circuit | есть `notifyWhileTrue` | нет встроенного `emitWhileTrue` |
| Жизненный цикл | intrusive, move-friendly observer/observable | weak slots удаляются при `emit` |
| Стиль API | строгий доменный контракт | гибкий callback API |

## Основные фичи Observable

- Move-only семантика источника и наблюдателя.
- Интрузивные подписки, безопасная отписка во время `notify`.
- Явный интерфейс событий (хорошо для архитектурных границ).
- `notifyWhileTrue` для цепочки проверок/фильтров.

## Основные фичи Signal

- Удобные подписки на разные типы callable.
- Дедупликация одинаковых слотов.
- Безопасная работа с `weak_ptr` (expired слоты очищаются в `emit`).
- Простая отписка по `Id` для лямбда-обработчиков.

## Один use case двумя способами

### 1) Через Observable (строгий контракт)

```cpp
#include "Awl/Observable.h"
#include "Awl/Observer.h"

struct ITemperatureEvents
{
    virtual void OnTemperature(int value) = 0;
};

class Sensor : public awl::Observable<ITemperatureEvents>
{
public:
    void Publish(int value)
    {
        notify(&ITemperatureEvents::OnTemperature, value);
    }
};

class UiPanel : public awl::Observer<ITemperatureEvents>
{
public:
    void OnTemperature(int value) override
    {
        last = value;
    }

    int last = 0;
};
```

### 2) Через Signal (callback API)

```cpp
#include "Awl/Signal.h"
#include <functional>

class Sensor2
{
public:
    awl::Signal<int> temperatureChanged;

    void Publish(int value)
    {
        temperatureChanged.emit(value);
    }
};

class UiPanel2
{
public:
    void OnTemperature(int value)
    {
        last = value;
    }

    int last = 0;
};

void Wire()
{
    Sensor2 s;
    UiPanel2 ui;

    s.temperatureChanged.subscribe(&ui, &UiPanel2::OnTemperature);

    int telemetry = 0;
    auto id = s.temperatureChanged.subscribe(std::function<void(int)>(
        [&telemetry](int v) { telemetry += v; }
    ));

    s.Publish(25);
    s.temperatureChanged.unsubscribe(id);
}
```

## Что выбрать

- Выбирайте `Observable`, если важны строгие интерфейсы и controlled emit внутри модели.
- Выбирайте `Signal`, если нужен быстрый интеграционный callback API с лямбдами и smart pointers.

