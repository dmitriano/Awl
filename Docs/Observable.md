# Observable (AWL)

## Что это

`awl::Observable<IObserver>` — это intrusive-механизм подписки по интерфейсу наблюдателя.
Подписчики реализуют интерфейс событий и наследуются от `awl::Observer<IObserver>`.

Ключевая идея: подписчик и источник событий можно безопасно перемещать (`move`), а подписки при этом сохраняются корректно.

## Основные фичи

- Move-only модель у `Observable` (copy запрещен, move разрешен).
- Move-only модель у `Observer`.
- Подписка/отписка через указатель на observer (`subscribe`, `unsubscribe`).
- Безопасная отписка из самого observer (`unsubscribeSelf`, `unsubscribeSafe`).
- `notify` вызывает метод интерфейса у всех подписчиков.
- `notifyWhileTrue` останавливает рассылку на первом `false`.
- Безопасная итерация при `notify`: observer может отписаться во время обработки.
- Защита API: `notify` обычно вызывается только внутри класса-источника (метод protected).

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

## Use case: валидация цепочкой через notifyWhileTrue

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

## Когда выбирать Observable

- Нужен строгий OOP-контракт событий через интерфейс.
- Нужна высокая предсказуемость поведения при `move` объектов.
- Важна безопасная отписка observer во время обработки событий.
- Нужен short-circuit сценарий (`notifyWhileTrue`).

