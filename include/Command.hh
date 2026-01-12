class Command
{
public:
  virtual ~Command() = default;
  virtual int execute() = 0;
};
