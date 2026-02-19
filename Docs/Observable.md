# Observable

`Observable` — это шаблонный класс для реализации паттерна Observer (Наблюдатель) в C++. Класс предоставляет типобезопасный механизм подписки-отписки наблюдателей и уведомлений о событиях.

## Основные характеристики

- **Типобезопасность**: Интерфейс наблюдателя определяется на этапе компиляции через шаблонный параметр `IObserver`
- **Move-only**: `Observable` не копируется, но поддерживает перемещение
- **Автоматическая отписка**: Наблюдатели автоматически отписываются при уничтожении
- **Безопасная модификация во время итерации**: Наблюдатели могут безопасно отписывать себя во время уведомления
- **Поддержка условий**: Метод `notifyWhileTrue` позволяет останавливать уведомления при выполнении условия
- **Intrusive контейнер**: Использует `quick_list` для эффективного хранения наблюдателей (1 указатель на наблюдателя)
- **Compile-time проверки**: Использует C++20 concepts для проверки типов аргументов
- **Дружественный доступ**: Параметр `Enclosing` позволяет предоставить доступ к защищённым методам классу-владельцу

## Архитектура

```
┌─────────────────────┐      quick_list      ┌──────────────────────┐
│   Observable<T>     │◄─────────────────────│ Observer<T>           │
│                     │                      │  (intrusive link)     │
│ - m_observers       │                      └──────────────────────┘
└─────────────────────┘                               ▲
                                                       │
                                            наследуется от
```

`Observable` использует intrusive linked list (`quick_list`), где каждый наблюдатель содержит узел списка. Это обеспечивает:
- O(1) подписку и отписку
- Минимальные накладные расходы на память
- Автоматическую очистку при уничтожении

## Заголовочный файл

```cpp
#include "Awl/Observable.h"
```

## Объявление

```cpp
template <class IObserver, class Enclosing = void>
class Observable;
```

### Параметры шаблона

| Параметр | Описание |
|----------|----------|
| `IObserver` | Интерфейс наблюдателя с виртуальными методами для обработки событий |
| `Enclosing` | Опциональный класс-владелец для доступа к защищённым методам (по умолчанию `void`) |

**Параметр `Enclosing`** позволяет указать дружественный класс, который получит доступ к защищённым методам `Observable` (в частности, к методам `notify`). Это полезно, когда вы хотите контролировать уведомления из внешнего класса:

```cpp
class Controller;

class Model : public awl::Observable<IModelEvents, Controller>
{
    // notify() доступен только из Controller и наследников
};

class Controller
{
    void notifyModelChanged(Model& model)
    {
        model.notify(&IModelEvents::onChange);
    }
};
```

## Основные методы

### Подписка и отписка

```cpp
void subscribe(Observer<IObserver>* p_observer);
void unsubscribe(Observer<IObserver>* p_observer);
bool empty() const;
auto size() const;
```

### Методы Observer

Наблюдатели, наследуемые от `Observer<IObserver>`, имеют дополнительные методы:

```cpp
// Проверка, подписан ли наблюдатель на какой-либо Observable
bool isSubscribed() const;

// Отписка от текущего Observable (безопасна для вызова из обработчика)
void unsubscribeSelf();

// Безопасная отписка с проверкой - ничего не делает, если не подписан
void unsubscribeSafe();
```

### Уведомление наблюдателей

```cpp
template<typename...Params, typename...Args>
void notify(void (IObserver::*func)(Params...), const Args&...args);

template<typename Result, typename...Params, typename...Args>
bool notifyWhileTrue(Result(IObserver::*func)(Params...), const Args&...args);
```

## Use Cases

### Use Case 1: Простой Observable

Базовый пример с одним наблюдателем:

```cpp
#include "Awl/Observable.h"
#include "Awl/Observer.h"

// 1. Определяем интерфейс наблюдателя
struct INotifyValueChanged
{
    virtual void onValueChanged(int newValue) = 0;
};

// 2. Создаём Observable
class Counter : public awl::Observable<INotifyValueChanged>
{
public:
    void setValue(int value)
    {
        m_value = value;
        // Уведомляем всех наблюдателей
        notify(&INotifyValueChanged::onValueChanged, m_value);
    }

private:
    int m_value = 0;
};

// 3. Создаём наблюдателя
class ValueDisplay : public awl::Observer<INotifyValueChanged>
{
public:
    void onValueChanged(int newValue) override
    {
        // Обрабатываем уведомление
    }
};

// Использование
int main()
{
    Counter counter;
    ValueDisplay display;

    counter.subscribe(&display);
    counter.setValue(42);  // Вызовет display.onValueChanged(42)
}
```

### Use Case 2: Несколько наблюдателей

```cpp
struct INotifyProgress
{
    virtual void onProgress(int percent) = 0;
};

class ProgressBar : public awl::Observable<INotifyProgress>
{
public:
    void setProgress(int percent)
    {
        notify(&INotifyProgress::onProgress, percent);
    }
};

class ConsoleLogger : public awl::Observer<INotifyProgress>
{
public:
    void onProgress(int percent) override
    {
        std::cout << "Progress: " << percent << "%" << std::endl;
    }
};

class FileLogger : public awl::Observer<INotifyProgress>
{
public:
    void onProgress(int percent) override
    {
        // Запись в файл
    }
};

// Использование
ProgressBar progress;
ConsoleLogger console;
FileLogger file;

progress.subscribe(&console);
progress.subscribe(&file);
progress.setProgress(50);  // Оба наблюдателя получат уведомление
```

### Use Case 3: Условное выполнение с notifyWhileTrue

Прекращение уведомлений при возвращении `false`:

```cpp
struct IValidator
{
    virtual bool validate(int value) = 0;
};

class DataProcessor : public awl::Observable<IValidator>
{
public:
    bool processData(int value)
    {
        // Продолжает уведомлять пока все возвращают true
        // Останавливается при первом false
        return notifyWhileTrue(&IValidator::validate, value);
    }
};

class RangeValidator : public awl::Observer<IValidator>
{
public:
    bool validate(int value) override
    {
        return value >= 0 && value <= 100;
    }
};

// Использование
DataProcessor processor;
RangeValidator validator;

processor.subscribe(&validator);
bool allValid = processor.processData(50);  // true
bool failed = processor.processData(150);    // false
```

### Use Case 4: Автоматическая отписка при уничтожении

```cpp
void example()
{
    Counter counter;
    auto display = std::make_unique<ValueDisplay>();

    counter.subscribe(display.get());
    // display получит уведомления

    display.reset();  // Автоматическая отписка
    // counter.size() == 0
}
```

### Use Case 5: Передача нескольких параметров

```cpp
struct INotifyTransaction
{
    virtual void onTransaction(const std::string& id, double amount, bool approved) = 0;
};

class BankAccount : public awl::Observable<INotifyTransaction>
{
public:
    void processTransaction(const std::string& id, double amount)
    {
        bool approved = amount <= balance;
        if (approved) balance -= amount;
        notify(&INotifyTransaction::onTransaction, id, amount, approved);
    }

private:
    double balance = 1000.0;
};
```

### Use Case 6: Использование с составными объектами

```cpp
struct Model
{
    awl::Observable<INotifyValueChanged> observable;
    awl::Observer<INotifyValueChanged> observer1;
    awl::Observer<INotifyValueChanged> observer2;

    Model()
    {
        observable.subscribe(&observer1);
        observable.subscribe(&observer2);
    }

    // Move-конструктор сохраняет подписки
    Model(Model&&) = default;
};
```

### Use Case 7: Проверка статуса подписки

```cpp
class SmartObserver : public awl::Observer<INotifyValueChanged>
{
public:
    void onValueChanged(int newValue) override
    {
        if (isSubscribed())
        {
            std::cout << "Still subscribed, value: " << newValue << "\n";
        }
    }

    void safeUnsubscribe()
    {
        unsubscribeSafe();  // Безопасно, даже если не подписаны
    }
};
```

### Use Case 8: Самоотписка из обработчика

```cpp
struct IOneShotEvent
{
    virtual void onEvent() = 0;
};

class OneShotHandler : public awl::Observer<IOneShotEvent>
{
public:
    void onEvent() override
    {
        std::cout << "Event received, unsubscribing\n";
        // Безопасно отписываем себя во время обработки
        unsubscribeSelf();
    }
};
```

## Compile-time проверки

`Observable` использует C++20 concepts для проверки типов аргументов на этапе компиляции:

```cpp
template<typename ...Params, typename ... Args>
void notify(void (IObserver::* func)(Params ...), const Args& ... args)
    requires (std::invocable<decltype(func), IObserver*, const Args&...>)
```

Это предотвращает ошибки несовпадения типов:

```cpp
struct IMyEvent
{
    virtual void onEvent(int value) = 0;
};

class MyObservable : public awl::Observable<IMyEvent>
{
public:
    void badNotify()
    {
        // Ошибка компиляции: string не преобразуется в int
        notify(&IMyEvent::onEvent, "wrong type");
    }

    void goodNotify()
    {
        // OK: типы совпадают
        notify(&IMyEvent::onEvent, 42);
    }
};
```

## Детали реализации

### Intrusive linked list

`Observable` использует `quick_list` — intrusive контейнер, где каждый наблюдатель содержит узел списка:

```cpp
template <class IObserver>
class Observer : public IObserver, public observer_link
```

**Преимущества:**
- Минимальные накладные расходы на память (1 указатель на наблюдателя)
- O(1) подписка и отписка
- Детерминированное время уведомления

**Недостатки:**
- Наблюдатель должен наследоваться от `Observer<IObserver>`
- Невозможно подписать произвольный объект без модификации

### Безопасная итерация

При уведомлении наблюдателей используется постфиксный инкремент для безопасного удаления во время итерации:

```cpp
for (typename ObserverList::iterator i = m_observers.begin(); i != m_observers.end(); )
{
    ObserverElement* p_observer = *(i++);  // Постфиксный инкремент!
    // p_observer может безопасно вызвать unsubscribeSelf()
    call(p_observer);
}
```

Это позволяет наблюдателям отписывать себя во время обработки уведомления.

### Автоматическая очистка

При уничтожении `Observable` все наблюдатели автоматически отписываются через `clearObservers()`:

```cpp
~ObservableImpl()
{
    clearObservers();  // Удаляет всех наблюдателей из списка
}
```

Это предотвращает обращение к уничтоженному списку при уничтожении наблюдателей.

## Связанные классы

- **`Observer<IObserver>`** — Базовый класс для наблюдателей
- **`quick_list`** — Внутренний контейнер для хранения наблюдателей (intrusive linked list)
- **`observer_link`** — Базовый класс для узла в `quick_list`

## Thread Safety

Класс `Observable` **не является потокобезопасным**. Используйте внешнюю синхронизацию при работе с несколькими потоками.

### Потокобезопасная обёртка

Для использования в многопоточной среде рекомендуется создать обёртку с мьютексом:

```cpp
#include <mutex>

template <class IObserver>
class ThreadSafeObservable
{
public:
    void subscribe(awl::Observer<IObserver>* observer)
    {
        std::lock_guard lock(m_mutex);
        m_observable.subscribe(observer);
    }

    void unsubscribe(awl::Observer<IObserver>* observer)
    {
        std::lock_guard lock(m_mutex);
        m_observable.unsubscribe(observer);
    }

    template<typename... Params, typename... Args>
    void notify(void (IObserver::*func)(Params...), const Args&... args)
    {
        std::lock_guard lock(m_mutex);
        m_observable.notify(func, args...);
    }

private:
    awl::Observable<IObserver> m_observable;
    std::mutex m_mutex;
};
```

## Best Practices

### 1. Используйте интерфейсы для типобезопасности

```cpp
// Хорошо: строгая типизация
struct IValueChanged
{
    virtual void onValueChanged(int oldValue, int newValue) = 0;
};

// Плохо: использование универсального интерфейса
struct IAnyEvent
{
    virtual void onEvent(int eventId, const void* data) = 0;
};
```

### 2. Не допускайте dangling pointers

```cpp
class BadExample
{
    awl::Observable<IMyEvent> observable;
    awl::Observer<IMyEvent> observer;

public:
    void setup()
    {
        observable.subscribe(&observer);
        // Ошибка: observer уничтожается раньше observable
    }
    // observable будет содержать dangling pointer!
};

class GoodExample
{
    awl::Observable<IMyEvent> observable;
    std::unique_ptr<awl::Observer<IMyEvent>> observer;

public:
    void setup()
    {
        observer = std::make_unique<MyObserver>();
        observable.subscribe(observer.get());
        // Порядок уничтожения: сначала observer, автоматическая отписка
    }
};
```

### 3. Используйте unsubscribeSafe() для условной отписки

```cpp
void cleanup()
{
    if (observer.isSubscribed())
    {
        observer.unsubscribeSafe();  // Безопасно, даже если не подписан
    }
}
```

## Ограничения

- **Move-only**: Класс не поддерживает копирование (copy semantics)
- **Наблюдатели должны существовать пока подписаны**: Уничтожение наблюдателя автоматически отписывает его, но dangling pointers могут возникнуть при неправильном порядке уничтожения
- **При перемещении Observable подписки сохраняются**: `Observable(Observable&&)` сохраняет всех наблюдателей
- **Один Observable на тип**: Наблюдатель может быть подписан только на один `Observable` данного типа (из-за intrusive списка)
- **Нет потокобезопасности**: Требуется внешняя синхронизация для многопоточного использования
