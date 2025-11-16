
namespace gc {

    class AbstractContainer {
public:
    virtual ~AbstractContainer() = default;
    virtual void grow() = 0;
    virtual void shrink() = 0;
    virtual void access() = 0;
};
}