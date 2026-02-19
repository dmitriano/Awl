# Observable vs Signal

Сравнение двух реализаций паттерна Observer в библиотеке AWL: `Observable` и `Signal`.

## Краткая сводка

| Характеристика | Observable | Signal |
|----------------|------------|--------|
| **Стиль** | Объектно-ориентированный | Функциональный |
| **Интерфейс подписчика** | Виртуальный класс (`IObserver`) | Функции (`equatable_function`) |
| **Хранение** | `quick_list` (intrusive) | `std::vector` |
| **Отписка** | Самостоятельная (`unsubscribeSelf`) | По ID или объекту |
| **Управление жизнью** | Ручное (наблюдатель сам следит) | `shared_ptr` / `weak_ptr` |
| **Копируемость** | Move-only | Move-only |
| **Заголовок** | `Awl/Observable.h` | `Awl/Signal.h` |

## Архитектурные различия

### Observable

```
┌─────────────────────┐      quick_list      ┌──────────────────────┐
│   Observable<T>     │◄─────────────────────│ Observer<T>           │
│                     │                      │  (intrusive link)     │
│ - m_observers       │                      └──────────────────────┘
└─────────────────────┘                               ▲
                                                       │
                                            наследуется от
```

**Ключевые особенности:**
- Использует intrusive linked list (`quick_list`)
- Наблюдатели наследуются от `Observer<IObserver>`
- Наблюдатель содержит ссылку на список (intrusive node)
- Отписка происходит через `unsubscribeSelf()`

### Signal

```
┌─────────────────────┐      std::vector      ┌──────────────────────┐
│   Signal<Args...>   │◄─────────────────────│ equatable_function   │
│                     │                      │  (type-erased call)  │
│ - m_slots           │                      └──────────────────────┘
└─────────────────────┘
```

**Ключевые особенности:**
- Использует `std::vector` для хранения слотов
- Подписчики - это функторы с type erasure
- Поддержка умных указателей (`shared_ptr`/`weak_ptr`)
- Автоматическая очистка недействительных `weak_ptr`

## Подробное сравнение

### 1. Интерфейс подписчика

#### Observable

Требует явного интерфейса с виртуальными методами:

```cpp
// 1. Определяем интерфейс
struct IValueChanged
{
    virtual void onValueChanged(int value) = 0;
};

// 2. Наблюдатель наследуется от Observer<IValueChanged>
class MyObserver : public awl::Observer<IValueChanged>
{
public:
    void onValueChanged(int value) override
    {
        std::cout << "Value: " << value << "\n";
    }
};
```

#### Signal

Работает с любым callable объектом:

```cpp
class MyClass
{
public:
    void onValueChange(int value)
    {
        std::cout << "Value: " << value << "\n";
    }
};

// Подписка без ограничений
MyClass obj;
Signal<int> sig;
sig.subscribe(&obj, &MyClass::onValueChange);

// Или с лямбдой
sig.subscribe([](int value) {
    std::cout << "Value: " << value << "\n";
});
```

### 2. Подписка и отписка

#### Observable

```cpp
// Подписка
observable.subscribe(&observer);

// Отписка (Observable вызывает unsubscribeSelf у наблюдателя)
observable.unsubscribe(&observer);

// Самоотписка изнутри обработчика
class MyObserver : public awl::Observer<IMyEvent>
{
public:
    void onEvent() override
    {
        // Можем отписать себя во время обработки
        unsubscribeSelf();
    }
};
```

#### Signal

```cpp
// Подписка с получением ID
awl::Id id = signal.subscribe([](int x) { /* ... */ });

// Отписка по ID
signal.unsubscribe(id);

// Подписка объекта
MyClass obj;
signal.subscribe(&obj, &MyClass::onEvent);

// Отписка по объекту и методу
signal.unsubscribe(&obj, &MyClass::onEvent);
```

### 3. Управление жизнью объектов

#### Observable

Наблюдатель должен сам управлять своим временем жизни. Использует intrusive list, где наблюдатель "знает" о своем нахождении в списке.

```cpp
void example()
{
    awl::Observable<IMyEvent> observable;
    auto observer = std::make_unique<MyObserver>();

    observable.subscribe(observer.get());

    // При уничтожении observer автоматически отписывается
    // благодаря intrusive list и деструктору Observer
}
```

#### Signal

Поддерживает умные указатели для автоматического управления:

```cpp
// weak_ptr - автоматическая отписка при уничтожении
{
    auto obj = std::make_shared<MyClass>();
    signal.subscribe(std::weak_ptr<MyClass>(obj), &MyClass::onEvent);
    // obj может быть уничтожен, Signal автоматически очистит слот
}

// shared_ptr - продление жизни объекта
{
    auto obj = std::make_shared<MyClass>();
    signal.subscribe(obj, &MyClass::onEvent);
    // Signal удерживает obj, пока есть подписка
}
```

### 4. Нотификация

#### Observable

```cpp
// Уведомление с вызовом конкретного метода
observable.notify(&IMyEvent::onValueChanged, 42);

// С условием - останавливается при первом false
bool allValid = observable.notifyWhileTrue(&IValidator::validate, data);
```

#### Signal

```cpp
// Простой эмит
signal.emit(42);

// Аргументы должны совпадать с шаблонными параметрами Signal
Signal<int, double> sig;
sig.emit(10, 3.14);
```

### 5. Безопасность при итерации

#### Observable

Использует postfix increment итератора для безопасного удаления во время итерации:

```cpp
// В ObservableImpl.h:
for (typename ObserverList::iterator i = m_observers.begin(); i != m_observers.end(); )
{
    ObserverElement* p_observer = *(i++);  // Постфиксный инкремент
    // p_observer может вызвать unsubscribeSelf() безопасно
    call(p_observer);
}
```

Наблюдатели могут безопасно отписывать себя во время уведомления.

#### Signal

Использует swap-and-pop стратегию при удалении недействительных слотов:

```cpp
// Удаление с перемещением последнего элемента
auto last = m_slots.end();
--last;
if (it != last)
{
    *it = std::move(*last);
}
m_slots.pop_back();
```

Слот с `weak_ptr` автоматически удаляется при неудачной попытке `lock()`.

## Когда использовать что

### Используйте Observable когда:

- **Требуется строгая типизация** через интерфейсы
- **Наблюдатель - сложный объект** с множеством методов
- **Нужна самоотписка** изнутри обработчика
- **Предпочитаете ООП-стиль** с виртуальными функциями
- **Важна детерминированная顺序** вызова наблюдателей
- **Наблюдатели переживают** объект Observable

**Пример:** Модель-представление в GUI, где представление должно реагировать на изменения модели

```cpp
struct IModelChanged
{
    virtual void onDataChanged() = 0;
    virtual void onStructureChanged() = 0;
};

class DataModel : public awl::Observable<IModelChanged>
{
    void setData() { notify(&IModelChanged::onDataChanged); }
    void restructure() { notify(&IModelChanged::onStructureChanged); }
};
```

### Используйте Signal когда:

- **Нужна гибкость** с лямбдами и функциями
- **Простые события** без сложной иерархии
- **Работа с умными указателями** (`shared_ptr`/`weak_ptr`)
- **Предпочитаете функциональный стиль**
- **Нужна автоматическая отписка** при уничтожении объекта
- **События - это отдельные сущности**, а не часть интерфейса

**Пример:** Кнопка с событием клика

```cpp
class Button
{
public:
    awl::Signal<> onClick;
    awl::Signal<int, int> onPositionChanged;  // x, y
};

Button btn;
btn.onClick.connect([]() { std::cout << "Clicked!\n"; });
```

## Таблица сравнения API

| Операция | Observable | Signal |
|----------|------------|--------|
| **Подписка** | `subscribe(ptr)` | `subscribe(func)` / `subscribe(ptr, method)` |
| **Отписка** | `unsubscribe(ptr)` | `unsubscribe(id)` / `unsubscribe(ptr, method)` |
| **Нотификация** | `notify(&I::method, args...)` | `emit(args...)` |
| **Условная нотификация** | `notifyWhileTrue(&I::method, args...)` | Не поддерживается |
| **Пустота** | `empty()` | `empty()` |
| **Размер** | `size()` | `size()` |
| **Очистка** | (через деструктор) | `clear()` |
| **Самоотписка** | `unsubscribeSelf()` | Через хранение ID |

## Производительность

| Аспект | Observable | Signal |
|--------|------------|--------|
| **Подписка** | O(1) - push_back в list | O(1) amortized - push_back в vector |
| **Отписка** | O(1) - intrusive unlink | O(1) - swap-and-pop (или O(n) для поиска) |
| **Нотификация** | O(n) - итерация по list | O(n) - итерация по vector |
| **Память на слот** | 1 указатель (intrusive) | ~24-32 байта (type-erased) |
| **Кэш-эффективность** | Ниже (linked list) | Выше (contiguous vector) |

## Зависимости

### Observable
- `Awl/Observer.h`
- `Awl/QuickList.h`
- `<concepts>`
- `<utility>`

### Signal
- `Awl/EquatableFunction.h`
- `Awl/UniqueId.h`
- `<algorithm>`
- `<concepts>`
- `<functional>`
- `<memory>`
- `<vector>`

## Взаимозаменяемость

Оба класса решают одну задачу, но разными способами. Выбор зависит от требований проекта:

```cpp
// Observable - более строгий, но безопаснее
class StrictModel : public awl::Observable<IModelEvents>
{
public:
    void update() { notify(&IModelEvents::onUpdate); }
};

// Signal - более гибкий, но требует больше дисциплины
class FlexibleModel
{
public:
    awl::Signal<> onUpdate;
};
```

Для существующих кодовых баз:

- **Qt-подобный код** → Signal (похож на Qt signals/slots)
- **Java/C#-подобный код** → Observable (похож на interfaces/events)
- **Функциональный стиль** → Signal
- **ООП-стиль** → Observable
