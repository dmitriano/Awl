# Observable

`Observable` — это шаблонный класс для реализации паттерна Observer (Наблюдатель) в C++. Класс предоставляет типобезопасный механизм подписки-отписки наблюдателей и уведомлений о событиях.

## Основные характеристики

- **Типобезопасность**: Интерфейс наблюдателя определяется на этапе компиляции через шаблонный параметр `IObserver`
- **Move-only**: `Observable` не копируется, но поддерживает перемещение
- **Автоматическая отписка**: Наблюдатели автоматически отписываются при уничтожении
- **Безопасная модификация во время итерации**: Наблюдатели могут безопасно отписывать себя во время уведомления
- **Поддержка условий**: Метод `notifyWhileTrue` позволяет останавливать уведомления при выполнении условия

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

## Основные методы

### Подписка и отписка

```cpp
void subscribe(Observer<IObserver>* p_observer);
void unsubscribe(Observer<IObserver>* p_observer);
bool empty() const;
auto size() const;
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

## Связанные классы

- **`Observer<IObserver>`** — Базовый класс для наблюдателей
- **`quick_list`** — Внутренний контейнер для хранения наблюдателей

## Thread Safety

Класс `Observable` **не является потокобезопасным**. Используйте внешнюю синхронизацию при работе с несколькими потоками.

## Ограничения

- Класс не поддерживает копирование (copy semantics)
- Наблюдатели должны существовать пока подписаны на Observable
- При перемещении Observable подписки сохраняются
