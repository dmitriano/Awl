# Observable (AWL)

## Что это

`awl::Observable<IObserver>` — это intrusive-механизм подписки по интерфейсу наблюдателя.
Подписчики реализуют интерфейс событий и наследуются от `awl::Observer<IObserver>`.

Ключевая идея: подписчик и источник событий можно безопасно перемещать (`move`), а подписки при этом сохраняются корректно.

## Основные фичи

- **Move-only модель источника и подписчика**: копирование отключено, перемещение разрешено.  
  Это снижает риск случайного дублирования подписок при копировании и делает поведение жизненного цикла более предсказуемым.
- **Intrusive-хранение подписок**: подписчик сам содержит link, а `Observable` хранит intrusive-список.  
  Практический эффект: нет отдельной heap-обертки на каждый subscription, операции подписки/отписки дешевле и проще по владению.
- **Простая явная подписка по адресу observer**: `subscribe(&observer)` / `unsubscribe(&observer)`.  
  Такой API хорошо подходит для моделей, где связи между объектами управляются явно.
- **Self-unsubscribe из observer**: `unsubscribeSelf()` и безопасный вариант `unsubscribeSafe()`.  
  Полезно, когда обработчик должен отписаться после первого события или по условию.
- **Типобезопасный `notify` через pointer-to-member**:  
  `notify(&IObserver::Method, args...)` проверяет сигнатуру на этапе компиляции, уменьшая риск runtime-ошибок.
- **`notifyWhileTrue` (short-circuit)**: остановка цепочки на первом `false`.  
  По сути это реализация паттерна **Chain of Responsibility**: каждый обработчик решает, передавать ли событие дальше.
  Удобно для пайплайнов проверок, фильтрации и механизмов "разрешить/запретить".
- **Безопасная итерация во время уведомления**: observer может отписаться в обработчике, и цикл рассылки останется корректным.
- **Контролируемый доступ к эмиту событий**: `notify` защищен (`protected`) и обычно вызывается только самим источником событий.  
  Это поддерживает инварианты доменной модели: внешние пользователи не могут "подделать" событие.
- **Предсказуемое поведение аргументов при рассылке**: в API используется передача аргументов как `const&`, что защищает от нежелательных эффектов perfect-forwarding при многократном вызове обработчиков.

## Базовый use case: доменные события

```cpp
#include "Awl/Observable.h"
#include "Awl/Observer.h"
#include <string>

struct IOrderEvents
{
    virtual void OnCreated(int id, const std::string& symbol) = 0;
    virtual void OnCancelled(int id) = 0;
};

class OrderBook : public awl::Observable<IOrderEvents>
{
public:
    void Create(int id, const std::string& symbol)
    {
        // notify доступен внутри класса-наследника
        notify(&IOrderEvents::OnCreated, id, symbol);
    }

    void Cancel(int id)
    {
        notify(&IOrderEvents::OnCancelled, id);
    }
};

class AuditLog : public awl::Observer<IOrderEvents>
{
public:
    void OnCreated(int id, const std::string& symbol) override
    {
        // логирование
    }

    void OnCancelled(int id) override
    {
        // логирование
    }
};

void Example()
{
    OrderBook book;
    AuditLog audit;

    book.subscribe(&audit);
    book.Create(42, "AAPL");
    book.Cancel(42);
    book.unsubscribe(&audit);
}
```

## Use case: Chain of Responsibility через notifyWhileTrue

```cpp
#include "Awl/Observable.h"
#include "Awl/Observer.h"

struct ICheck
{
    virtual bool Check(int value) = 0;
};

class ValidationBus : public awl::Observable<ICheck>
{
public:
    bool Validate(int value)
    {
        // Остановится на первом false
        return notifyWhileTrue(&ICheck::Check, value);
    }
};

class PositiveCheck : public awl::Observer<ICheck>
{
public:
    bool Check(int value) override { return value > 0; }
};
```

## Use case: `Observable` мувается при real-world релокации контейнера

Ниже сценарий, где `move` происходит не вручную, а автоматически:  
объекты хранятся в `std::vector`, и при росте емкости вектор перемещает элементы.

```cpp
#include "Awl/Observable.h"
#include "Awl/Observer.h"
#include <vector>

struct IHealthEvents
{
    virtual void OnHealthChanged(int hp) = 0;
};

class HealthChannel : public awl::Observable<IHealthEvents>
{
public:
    void Emit(int hp)
    {
        notify(&IHealthEvents::OnHealthChanged, hp);
    }
};

class HealthHud : public awl::Observer<IHealthEvents>
{
public:
    void OnHealthChanged(int hp) override
    {
        lastHp = hp;
    }

    int lastHp = 0;
};

class Unit
{
public:
    Unit()
    {
        // Внутренняя подписка "канал -> HUD"
        healthEvents.subscribe(&hud);
    }

    Unit(const Unit&) = delete;
    Unit(Unit&&) = default;
    Unit& operator=(const Unit&) = delete;
    Unit& operator=(Unit&&) = default;

    void Damage(int value)
    {
        hp -= value;
        healthEvents.Emit(hp);
    }

    int LastHudHp() const
    {
        return hud.lastHp;
    }

private:
    int hp = 100;
    HealthChannel healthEvents;
    HealthHud hud;
};

void ExampleVectorRelocation()
{
    std::vector<Unit> units;
    units.reserve(1);

    units.emplace_back();
    units[0].Damage(10); // HUD видит 90

    // Здесь обычно происходит real-world move первого элемента из-за reallocation.
    units.emplace_back();

    // Подписка сохраняет корректность после перемещения Unit.
    units[0].Damage(5);  // HUD первого Unit видит 85
}
```

Почему это важно: в моделях/сценах с контейнерами сущностей `move` часто происходит неявно (reallocation, reordering, swap), и `Observable` в AWL рассчитан на такие сценарии.

## Когда выбирать Observable

- Нужен строгий OOP-контракт событий через интерфейс.
- Нужна высокая предсказуемость поведения при `move` объектов.
- Важна безопасная отписка observer во время обработки событий.
- Нужен short-circuit сценарий (`notifyWhileTrue`).
