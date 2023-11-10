// This class provides the callbacks that occur when parsing input assembly.
class MCStreamerWrapper final : public MCStreamer {
  CodeRegions &Regions;

public:
  MCStreamerWrapper(MCContext &Context, mca::CodeRegions &R)
      : MCStreamer(Context), Regions(R) {}

  // We only want to intercept the emission of new instructions.
  virtual void emitInstruction(const MCInst &Inst,
                               const MCSubtargetInfo & /* unused */) override {
    // IPU local patch begin
    if (Inst.getOpcode() == TargetOpcode::BUNDLE) {
      // if instruction is a bundle, add inner instructions to region
      for (unsigned i = 0; i < Inst.getNumOperands(); i++) {

        assert(Inst.getOperand(i).isInst() &&
               "Expect operand of a bundle to be an MCInst");

        Regions.addInstruction(*(Inst.getOperand(i).getInst()));
      }
    }
    // IPU local patch end
    Regions.addInstruction(Inst);
  }

  bool emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override {
    return true;
  }

  void emitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                        unsigned ByteAlignment) override {}
  void emitZerofill(MCSection *Section, MCSymbol *Symbol = nullptr,
                    uint64_t Size = 0, unsigned ByteAlignment = 0,
                    SMLoc Loc = SMLoc()) override {}
  void emitGPRel32Value(const MCExpr *Value) override {}
  void beginCOFFSymbolDef(const MCSymbol *Symbol) override {}
  void emitCOFFSymbolStorageClass(int StorageClass) override {}
  void emitCOFFSymbolType(int Type) override {}
  void endCOFFSymbolDef() override {}

  ArrayRef<MCInst> GetInstructionSequence(unsigned Index) const {
    return Regions.getInstructionSequence(Index);
  }
};