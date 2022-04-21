#pragma once

#include "Bases.hpp"

namespace sp{


constexpr size_t CacheSize = 32768;
constexpr size_t CachePage = 64;

constexpr size_t CacheAvalible = CacheSize / 2;
constexpr size_t CacheBlockLen = int_sqrt(CacheAvalible);



template<class Base>
struct MatrixWrapper : Base{

	template<SP_MATRIX_T(M)>
	const MatrixWrapper &operator =(M &&rhs) noexcept{
		resize(*this, rows(rhs), cols(rhs));
		if constexpr (std::decay_t<M>::UndefMajor ? Base::RowMajor : std::decay_t<M>::RowMajor)
			for (size_t i=0; i!=rows(*this); ++i)
				for (size_t j=0; j!=cols(*this); ++j)
					(*this)(i, j) = rhs(i, j);
		else
			for (size_t i=0; i!=cols(*this); ++i)
				for (size_t j=0; j!=rows(*this); ++j)
					(*this)(j, i) = rhs(j, i);
		if constexpr (std::decay_t<M>::UsesBuffer)
			resize(MatrixTempStorage.data, MatrixTempStorage.stack_pos);
		return *this;
	}

	template<SP_MATRIX_T(M)>
	const MatrixWrapper &operator +=(M &&rhs) noexcept{
		for (size_t i=0; i!=rows(*this); ++i)
			for (size_t j=0; j!=cols(*this); ++j)
				(*this)(i, j) += rhs(i, j);
		return *this;
	}

	template<SP_MATRIX_T(M)>
	const MatrixWrapper &operator -=(M &&rhs) noexcept{
		for (size_t i=0; i!=rows(*this); ++i)
			for (size_t j=0; j!=cols(*this); ++j)
				(*this)(i, j) -= rhs(i, j);
		return *this;
	}

	template<SP_MATRIX_T(M)>
	const MatrixWrapper &operator *=(M &&rhs) noexcept{
		resize(*this, rows(*this), cols(rhs));

		size_t oldSize = len(MatrixTempStorage.data);
		expand_back(
			MatrixTempStorage.data,
			(len(*this) * sizeof(typename Base::ValueType) + 7) / 8
		);
		typename Base::ValueType *TempStorage =
				(typename Base::ValueType *)beg(MatrixTempStorage.data) + oldSize;
		
		typename Base::ValueType *J = TempStorage;
		for (const auto I=beg(*this); I!=end(*this); ++I, ++J) *J = *I;
		if constexpr (Base::RowMajor){
			for (size_t i=0; i!=cols(*this); ++i)
				for (size_t j=0; j!=rhs.rows(); ++j)
					for (size_t k=0; k!=rows(*this); ++k)
						(*this)(i, j) = TempStorage[i*cols(*this)+k] * rhs(j, k);
		} else{
			for (size_t i=0; i!=cols(*this); ++i)
				for (size_t j=0; j!=rhs.rows(); ++j)
					for (size_t k=0; k!=rows(*this); ++k)
						(*this)(i, j) = TempStorage[i+k*cols(*this)] * rhs(j, k);
		}
			
		resize(MatrixTempStorage.data, oldSize);
		return *this;
	}
	
};


template<class M, bool isLVal>
struct MatrixExprTranspose{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool RowMajor = !Arg::RowMajor;
	constexpr static bool UndefMajor = Arg::UndefMajor;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	SP_CI std::conditional_t<isLVal, ValueType &, ValueType> operator ()(
		size_t r, size_t c
	) noexcept{ return arg(c, r); }
	
	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{ return arg(c, r); }
};

template<class M, bool isLVal>
SP_CSI size_t rows(const MatrixExprTranspose<M, isLVal> &m) noexcept{ return cols(m.arg); }

template<class M, bool isLVal>
SP_CSI size_t cols(const MatrixExprTranspose<M, isLVal> &m) noexcept{ return rows(m.arg); }

template<class M, bool isLVal>
SP_CSI size_t len(const MatrixExprTranspose<M, isLVal> &m) noexcept{ return len(m.arg); }

template<class M, bool isLVal>
SP_CSI size_t cap(const MatrixExprTranspose<M, isLVal> &m) noexcept{ return cap(m.arg); }

template<class M, bool isLVal>
SP_SI void resize(MatrixExprTranspose<M, isLVal> &m, size_t r, size_t c) noexcept{
	resize(m.arg, c, r);
};



template<class M, class Cont, bool isLVal>
struct MatrixExprPermuteRows{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;
	const Cont *permuts;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool IsExpr = Arg::IsExpr;
	constexpr static bool RowMajor = Arg::RowMajor;
	constexpr static bool UndefMajor = Arg::UndefMajor;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	SP_CI std::conditional_t<isLVal, ValueType &, ValueType> operator ()(
		size_t r, size_t c
	) noexcept{ return arg((*permuts)[r], c); }
	
	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{
		return arg((*permuts)[r], c);
	}
};

template<class M, class Cont, bool isLVal>
SP_CSI size_t rows(const MatrixExprPermuteRows<M, Cont, isLVal> &m) noexcept{
	return rows(m.arg);
}

template<class M, class Cont, bool isLVal>
SP_CSI size_t cols(const MatrixExprPermuteRows<M, Cont, isLVal> &m) noexcept{
	return cols(m.arg);
}

template<class M, class Cont, bool isLVal>
SP_CSI size_t len(const MatrixExprPermuteRows<M, Cont, isLVal> &m) noexcept{ return len(m.arg); }

template<class M, class Cont, bool isLVal>
SP_CSI size_t cap(const MatrixExprPermuteRows<M, Cont, isLVal> &m) noexcept{ return cap(m.arg); }

template<class M, class Cont, bool isLVal>
SP_SI size_t resize(MatrixExprPermuteRows<M, Cont, isLVal> &m, size_t r, size_t c) noexcept{
	return resize(m.arg, r, c);
}



template<class M, class Cont, bool isLVal>
struct MatrixExprPermuteCols{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;
	const Cont *permuts;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool IsExpr = Arg::IsExpr;
	constexpr static bool RowMajor = Arg::RowMajor;
	constexpr static bool UndefMajor = Arg::UndefMajor;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	SP_CI std::conditional_t<isLVal, ValueType &, ValueType> operator ()(
		size_t r, size_t c
	) noexcept{ return arg(r, (*permuts)[c]); }
	
	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{
		return arg(r, (*permuts)[c]);
	}
};

template<class M, class Cont, bool isLVal>
SP_CSI size_t rows(const MatrixExprPermuteCols<M, Cont, isLVal> &m) noexcept{
	return rows(m.arg);
}

template<class M, class Cont, bool isLVal>
SP_CSI size_t cols(const MatrixExprPermuteCols<M, Cont, isLVal> &m) noexcept{
	return cols(m.arg);
}

template<class M, class Cont, bool isLVal>
SP_CSI size_t len(const MatrixExprPermuteCols<M, Cont, isLVal> &m) noexcept{ return len(m.arg); }

template<class M, class Cont, bool isLVal>
SP_CSI size_t cap(const MatrixExprPermuteCols<M, Cont, isLVal> &m) noexcept{ return cap(m.arg); }

template<class M, class Cont, bool isLVal>
SP_SI size_t resize(MatrixExprPermuteCols<M, Cont, isLVal> &m, size_t r, size_t c) noexcept{
	return resize(m.arg, r, c);
}



template<class M, auto Operation>
struct MatrixExprElStatUnaryOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool RowMajor = Arg::RowMajor;
	constexpr static bool UndefMajor = Arg::UndefMajor;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	template<class O = decltype(Operation)> SP_CI
	std::enable_if_t<std::is_invocable_v<O, ValueType>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{
		return Operation(arg(r, c));
	}
	
	template<class O = decltype(Operation)> SP_CI
	std::enable_if_t<std::is_invocable_v<O, ValueType, size_t, size_t>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{
		return Operation(arg(r, c), r, c);
	}
};

template<class M, auto Op>
SP_CSI size_t rows(const MatrixExprElStatUnaryOp<M, Op> &m) noexcept{
	return rows(m.arg.rows);
}

template<class M, auto Op>
SP_CSI size_t cols(const MatrixExprElStatUnaryOp<M, Op> &m) noexcept{
	return cols(m.arg.cols);
}

template<class M, auto Op>
SP_CSI size_t len(const MatrixExprElStatUnaryOp<M, Op> &m) noexcept{
	return len(m.arg.len);
}



template<class M, class Operation>
struct MatrixExprElDynUnaryOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;
	Operation operation;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool RowMajor = Arg::RowMajor;
	constexpr static bool UndefMajor = Arg::UndefMajor;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	template<class O = Operation> SP_CI
	std::enable_if_t<std::is_invocable_v<O, ValueType>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{ 
		return operation(arg(r, c));
	}
	
	template<class O = Operation> SP_CI
	std::enable_if_t<std::is_invocable_v<O, ValueType, size_t, size_t>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{ 
		return operation(arg(r, c), r, c);
	}
};

template<class M, class Operation>
SP_CSI size_t rows(const MatrixExprElDynUnaryOp<M, Operation> &m) noexcept{
	return rows(m.arg.rows);
}

template<class M, class Operation>
SP_CSI size_t cols(const MatrixExprElDynUnaryOp<M, Operation> &m) noexcept{
	return cols(m.arg.cols);
}

template<class M, class Operation>
SP_CSI size_t len(const MatrixExprElDynUnaryOp<M, Operation> &m) noexcept{
	return len(m.arg.len);
}



template<class T, bool rowMaj>
struct MatrixExprCopy{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	size_t data_index;
	uint32_t rows;
	uint32_t cols;

	template<SP_MATRIX_T(M)>
	MatrixExprCopy(M &&A) noexcept :
		data_index(len(MatrixTempStorage.data)), rows(A.rows()), cols(A.cols())
	{
		expand_back(MatrixTempStorage.data, (len(*this) * sizeof(T) + 7) / 8);
		T *I = (T *)beg(MatrixTempStorage.data) + data_index;
		if constexpr (rowMaj)
			for (size_t i=0; i!=rows; ++i)
				for (size_t j=0; j!=cols; ++j, ++I)
					*I = A(i, j);
		else
			for (size_t i=0; i!=cols; ++i)
				for (size_t j=0; j!=rows; ++j, ++I)
					*I = A(j, i);
	}


	typedef T ValueType;
	constexpr static bool RowMajor = rowMaj;
	constexpr static bool UndefMajor = false;
	constexpr static bool UsesBuffer = true;

	SP_CI T operator ()(size_t r, size_t c) const noexcept{
		if constexpr (RowMajor)
			return *((T *)beg(MatrixTempStorage.data)+data_index + r*(size_t)cols + c);
		else
			return *((T *)beg(MatrixTempStorage.data)+data_index + r + c*(size_t)rows);
	}
};

template<class M, bool rowMaj>
SP_CSI size_t rows(const MatrixExprCopy<M, rowMaj> &m) noexcept{
	return rows(m.arg.rows);
}

template<class M, bool rowMaj>
SP_CSI size_t cols(const MatrixExprCopy<M, rowMaj> &m) noexcept{
	return cols(m.arg.cols);
}

template<class M, bool rowMaj>
SP_CSI size_t len(const MatrixExprCopy<M, rowMaj> &m) noexcept{
	return len(m.arg.len);
}

template<class M, bool rowMaj>
SP_CSI size_t cap(const MatrixExprCopy<M, rowMaj> &m) noexcept{
	return len(m.arg);
};



template<class ML, class MR>
struct MatrixExprAdd{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	ML lhs;
	MR rhs;

	typedef std::remove_reference_t<ML> Lhs;
	typedef std::remove_reference_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool RowMajor = Lhs::RowMajor;
	constexpr static bool UndefMajor =
			Lhs::UndefMajor || Rhs::UndefMajor || Lhs::RowMajor==Rhs::RowMajor;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{
		return lhs(r, c) + rhs(r, c);
	}
};

template<class ML, class MR>
SP_CSI size_t rows(const MatrixExprAdd<ML, MR> &m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
SP_CSI size_t cols(const MatrixExprAdd<ML, MR> &m) noexcept{ return cols(m.lhs); }

template<class ML, class MR>
SP_CSI size_t len(const MatrixExprAdd<ML, MR> &m) noexcept{ return len(m.lhs); }



template<class ML, class MR>
struct MatrixExprSubtract{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	ML lhs;
	MR rhs;

	typedef std::remove_reference_t<ML> Lhs;
	typedef std::remove_reference_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool RowMajor = Lhs::RowMajor;
	constexpr static bool UndefMajor =
			Lhs::UndefMajor || Rhs::UndefMajor || Lhs::RowMajor==Rhs::RowMajor;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{
		return lhs(r, c) - rhs(r, c);
	}
};

template<class ML, class MR>
SP_CSI size_t rows(const MatrixExprSubtract<ML, MR> &m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
SP_CSI size_t cols(const MatrixExprSubtract<ML, MR> &m) noexcept{ return cols(m.lhs); }

template<class ML, class MR>
SP_CSI size_t len(const MatrixExprSubtract<ML, MR> &m) noexcept{ return len(m.lhs); }



template<class ML, class MR>
struct MatrixExprElMul{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	ML lhs;
	MR rhs;

	typedef std::remove_reference_t<ML> Lhs;
	typedef std::remove_reference_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool RowMajor = Lhs::RowMajor;
	constexpr static bool UndefMajor =
			Lhs::UndefMajor || Rhs::UndefMajor || Lhs::RowMajor==Rhs::RowMajor;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{
		return lhs(r, c) * rhs(r, c);
	}
};

template<class ML, class MR>
SP_CSI size_t rows(const MatrixExprElMul<ML, MR> &m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
SP_CSI size_t cols(const MatrixExprElMul<ML, MR> &m) noexcept{ return cols(m.lhs); }

template<class ML, class MR>
SP_CSI size_t len(const MatrixExprElMul<ML, MR> &m) noexcept{ return len(m.lhs); }




template<class ML, class MR>
struct MatrixExprElDiv{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	ML lhs;
	MR rhs;

	typedef std::remove_reference_t<ML> Lhs;
	typedef std::remove_reference_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool RowMajor = Lhs::RowMajor;
	constexpr static bool UndefMajor =
			Lhs::UndefMajor || Rhs::UndefMajor || Lhs::RowMajor==Rhs::RowMajor;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{
		return lhs(r, c) / rhs(r, c);
	}
};

template<class ML, class MR>
SP_CSI size_t rows(const MatrixExprElDiv<ML, MR> &m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
SP_CSI size_t cols(const MatrixExprElDiv<ML, MR> &m) noexcept{ return cols(m.lhs); }

template<class ML, class MR>
SP_CSI size_t len(const MatrixExprElDiv<ML, MR> &m) noexcept{ return len(m.lhs); }



template<class ML, class MR, auto Operation>
struct MatrixExprElStatOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	ML lhs;
	MR rhs;

	typedef std::remove_reference_t<ML> Lhs;
	typedef std::remove_reference_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool RowMajor = Lhs::RowMajor;
	constexpr static bool UndefMajor =
			Lhs::UndefMajor || Rhs::UndefMajor || Lhs::RowMajor==Rhs::RowMajor;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	template<class O = decltype(Operation)> SP_CI
	std::enable_if_t<std::is_invocable_v<O, ValueType>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{
		return Op(lhs(r, c), rhs(r, c));
	}
	
	template<class O = decltype(Operation)> SP_CI
	std::enable_if_t<std::is_invocable_v<O, ValueType, ValueType, size_t, size_t>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{
		return Op(lhs(r, c), rhs(r, c), r, c);
	}
};

template<class ML, class MR, auto Op>
SP_CSI size_t rows(const MatrixExprElStatOp<ML, MR, Op> &m) noexcept{ return rows(m.lhs); }

template<class ML, class MR, auto Op>
SP_CSI size_t cols(const MatrixExprElStatOp<ML, MR, Op> &m) noexcept{ return cols(m.lhs); }

template<class ML, class MR, auto Op>
SP_CSI size_t len(const MatrixExprElStatOp<ML, MR, Op> &m) noexcept{ return len(m.lhs); }



template<class ML, class MR, class Operation>
struct MatrixExprElDynOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	ML lhs;
	MR rhs;
	Operation operation;

	typedef std::remove_reference_t<ML> Lhs;
	typedef std::remove_reference_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool RowMajor = Lhs::RowMajor;
	constexpr static bool UndefMajor =
			Lhs::UndefMajor || Rhs::UndefMajor || Lhs::RowMajor==Rhs::RowMajor;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	
	template<class O = Operation> SP_CI
	std::enable_if_t<std::is_invocable_v<O, ValueType, ValueType>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{
		return operation(lhs(r, c), rhs(r, c));
	}
	
	template<class O = Operation> SP_CI
	std::enable_if_t<std::is_invocable_v<O, ValueType, ValueType, size_t, size_t>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{
		return operation(lhs(r, c), rhs(r, c), r, c);
	}
};

template<class ML, class MR, class Op>
SP_CSI size_t rows(const MatrixExprElDynOp<ML, MR, Op> &m) noexcept{ return rows(m.lhs); }

template<class ML, class MR, class Op>
SP_CSI size_t cols(const MatrixExprElDynOp<ML, MR, Op> &m) noexcept{ return cols(m.lhs); }

template<class ML, class MR, class Op>
SP_CSI size_t len(const MatrixExprElDynOp<ML, MR, Op> &m) noexcept{ return len(m.lhs); }



template<class ML, class MR>
struct MatrixExprMultiply{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	ML lhs;
	MR rhs;

	typedef std::remove_reference_t<ML> Lhs;
	typedef std::remove_reference_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool RowMajor = false;
	constexpr static bool UndefMajor = true;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{
		ValueType res = (ValueType)0;
		for (size_t i=0; i!=cols(lhs); ++i)
			res += lhs(r, i) * rhs(i, c);
		return res;
	}
};

template<class ML, class MR>
SP_CSI size_t rows(const MatrixExprMultiply<ML, MR> &m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
SP_CSI size_t cols(const MatrixExprMultiply<ML, MR> &m) noexcept{ return cols(m.rhs); }

template<class ML, class MR>
SP_CSI size_t len(const MatrixExprMultiply<ML, MR> &m) noexcept{
	return rows(m.lhs) * cols(m.rhs);
}



template<class M>
struct MatrixExprScalarMultiply{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	typedef std::remove_reference_t<M> Arg;
	typedef typename M::ValueType ValueType;

	M lhs;
	typename M::ValueType rhs;

	constexpr static bool RowMajor = Arg::RowMajor;
	constexpr static bool UndefMajor = Arg::UndefMajor;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{
		return lhs(r, c) * rhs;
	}
};

template<class M>
SP_CSI size_t rows(const MatrixExprScalarMultiply<M> &m) noexcept{ return rows(m.lhs); }

template<class M>
SP_CSI size_t cols(const MatrixExprScalarMultiply<M> &m) noexcept{ return cols(m.lhs); }

template<class M>
SP_CSI size_t len(const MatrixExprScalarMultiply<M> &m) noexcept{ return len(m.lhs); }

// there's no divide, because it's slow



template<class O>
SP_CSI auto matrix_generator_helper_impl_(O &&op){
	if constexpr (std::is_invocable_v<O, size_t, size_t>)
		return op(0, 0);
	if constexpr (std::is_invocable_v<O>)
		return op();
	else
		static_assert("generating function has wrong parameter types");
}

template<auto Operation>
struct MatrixExprStatGenerator{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	uint32_t rows;
	uint32_t cols;
	
	typedef decltype(matrix_generator_helper_impl_(Operation)) ValueType;
	constexpr static bool RowMajor = true;
	constexpr static bool UndefMajor = true;
	constexpr static bool UsesBuffer = false;

	template<class O = decltype(Operation)> SP_CI
	std::enable_if_t<std::is_invocable_v<O, size_t, size_t>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{ return Operation(r, c); }
	
	template<class O = decltype(Operation)> SP_CI
	std::enable_if_t<std::is_invocable_v<O>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{ return Operation(); }
};

template<auto Op>
SP_CSI size_t rows(const MatrixExprStatGenerator<Op> &m) noexcept{ return m.rows; }

template<auto Op>
SP_CSI size_t cols(const MatrixExprStatGenerator<Op> &m) noexcept{ return m.cols; }

template<auto Op>
SP_CSI size_t len(const MatrixExprStatGenerator<Op> &m) noexcept{ return m.rows * m.cols; }



template<class Operation>
struct MatrixExprDynGenerator{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	uint32_t rows;
	uint32_t cols;
	Operation operation;

	typedef decltype(matrix_generator_helper_impl_(operation)) ValueType;
	constexpr static bool RowMajor = true;
	constexpr static bool UndefMajor = true;
	constexpr static bool UsesBuffer = false;

	template<class O = Operation> SP_CI
	std::enable_if_t<std::is_invocable_v<O, size_t, size_t>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{ return operation(r, c); }
	
	template<class O = Operation> SP_CI
	std::enable_if_t<std::is_invocable_v<O>, ValueType>
	operator ()(size_t r, size_t c) const noexcept{ return operation(); }
};

template<class Op>
SP_CSI size_t rows(const MatrixExprDynGenerator<Op> &m) noexcept{ return m.rows; }

template<class Op>
SP_CSI size_t cols(const MatrixExprDynGenerator<Op> &m) noexcept{ return m.cols; }

template<class Op>
SP_CSI size_t len(const MatrixExprDynGenerator<Op> &m) noexcept{ return m.rows * m.cols; }



template<class T>
struct MatrixExprUniformValue{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	uint32_t rows;
	uint32_t cols;
	T value;

	typedef T ValueType;
	constexpr static bool RowMajor = true;
	constexpr static bool UndefMajor = true;
	constexpr static bool UsesBuffer = false;

	SP_CI ValueType operator ()(size_t r, size_t c) const noexcept{ return value; }
};

template<class T>
SP_CSI size_t rows(const MatrixExprUniformValue<T> &m) noexcept{ return m.rows; }

template<class T>
SP_CSI size_t cols(const MatrixExprUniformValue<T> &m) noexcept{ return m.cols; }

template<class T>
SP_CSI size_t len(const MatrixExprUniformValue<T> &m) noexcept{ return m.rows * m.cols; }






template<SP_MATRIX_T(M)>
auto tr(M &&arg) noexcept{
	return MatrixExprTranspose<CRemRRef<M>, false>{arg};
}
template<SP_MATRIX_T(M)>
auto l_tr(M &&arg) noexcept{
	return MatrixWrapper<MatrixExprTranspose<RemRRef<M>, true>>{{arg}};
}


template<SP_MATRIX_T(M), class Cont>
auto perm_rows(M &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(rows(arg) != len(permuts),
		"permutation array must have the same length as number of permuted matrix's rows"
	);
	return MatrixExprPermuteRows<CRemRRef<M>, Cont, false>{arg, &permuts};
}
template<SP_MATRIX_T(M), class Cont>
auto l_perm_rows(M &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(rows(arg) != len(permuts),
		"permutation array must have the same length as number of permuted matrix's rows"
	);
	return MatrixWrapper<MatrixExprPermuteRows<RemRRef<M>, Cont, true>>{{arg, &permuts}};
}

template<SP_MATRIX_T(M), class Cont>
auto perm_cols(M &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(cols(arg) != len(permuts),
		"permutation array must have the same length as number of permuted matrix's columns"
	);
	return MatrixExprPermuteCols<CRemRRef<M>, Cont, false>{arg, &permuts};
}
template<SP_MATRIX_T(M), class Cont>
auto l_permCols(M &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(cols(arg) != len(permuts),
		"permutation array must have the same length as number of permuted matrix's columns"
	);
	return MatrixWrapper<MatrixExprPermuteCols<RemRRef<M>, Cont, true>>{{arg, &permuts}};
}

template<SP_MATRIX_T(M)>
auto cp(const M &arg) noexcept{
	return MatrixExprCopy<typename std::decay_t<M>::ValueType, M::RowMajor>{arg};
}

template<auto operation, SP_MATRIX_T(M)>
auto apply(M &&arg) noexcept{
	return MatrixExprElStatUnaryOp<CRemRRef<M>, operation>{arg};
}

template<SP_MATRIX_T(M), class Operation>
auto apply(M &&arg, Operation &&operation) noexcept{
	return MatrixExprElDynUnaryOp<
		CRemRRef<M>, decltype((Operation &&)operation)
	>{arg, (Operation &&)operation};
}

template<SP_MATRIX_T(ML), SP_MATRIX_T(MR)>
auto elwise_mul(ML &&lhs, MR &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise multiplied matrices cannot have different dimensions"
	);
	return MatrixExprElMul<CRemRRef<ML>, CRemRRef<MR>>{lhs, rhs};
}

template<SP_MATRIX_T(ML), SP_MATRIX_T(MR)>
auto elwise_div(ML &&lhs, MR &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise divided matrices cannot have different dimensions"
	);
	return MatrixExprElDiv<CRemRRef<ML>, CRemRRef<MR>>{lhs, rhs};
}

template<SP_MATRIX_T(ML), SP_MATRIX_T(MR), class Operation>
auto apply(ML &&lhs, MR &&rhs, Operation &&operation) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExprElDynOp<CRemRRef<ML>, CRemRRef<MR>,
			decltype((Operation &&)operation)>{
				lhs, rhs, (Operation &&)operation
			};
}

template<auto operation, SP_MATRIX_T(ML), SP_MATRIX_T(MR)>
auto apply(ML &&lhs, MR &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExprElStatOp<CRemRRef<ML>, CRemRRef<MR>, operation>{lhs, rhs};
}






template<auto Operation>
auto generate(uint32_t rows, uint32_t cols) noexcept{
	return MatrixExprStatGenerator<Operation>{rows, cols};
}

template<class Operation>
auto generate(
	uint32_t rows, uint32_t cols, Operation &&operation
) noexcept{
	return MatrixExprDynGenerator<decltype((Operation &&)operation)>{
		rows, cols, (Operation &&)operation
	};
}

template<class T>
auto uniform(uint32_t rows, uint32_t cols, T value) noexcept{
	return MatrixExprUniformValue<T>{rows, cols, value};
}


template<SP_MATRIX_T(ML), SP_MATRIX_T(MR)>
auto operator +(ML &&lhs, MR &&rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"added matrices cannot have different dimensions"
	);
	return MatrixExprAdd<CRemRRef<ML>, CRemRRef<MR>>{lhs, rhs};	
}

template<SP_MATRIX_T(ML), SP_MATRIX_T(MR)>
auto operator -(ML &&lhs, MR &&rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"subtracted matrices cannot have different dimensions"
	);
	return MatrixExprSubtract<CRemRRef<ML>, CRemRRef<MR>>{lhs, rhs};	
}

template<SP_MATRIX_T(ML), SP_MATRIX_T(MR)>
auto operator *(ML &&lhs, MR &&rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != rows(rhs), "multiplied matrices have wrong dimensionss");
	return MatrixExprMultiply<CRemRRef<ML>, CRemRRef<MR>>{lhs, rhs};
}



template<SP_MATRIX_T(M)>
auto operator *(M &&lhs, const typename std::decay_t<M>::ValueType &rhs) noexcept{
	return MatrixExprScalarMultiply<CRemRRef<M>>{lhs, rhs};
}
template<SP_MATRIX_T(M)>
auto operator *(const typename std::decay_t<M>::ValueType &lhs, M &&rhs) noexcept{
	return MatrixExprScalarMultiply<CRemRRef<M>>{rhs, lhs};
}

template<SP_MATRIX_T(M)>
auto operator /(M &&lhs, const typename std::decay_t<M>::ValueType &rhs) noexcept{
	return MatrixExprScalarMultiply<CRemRRef<M>>{lhs, (typename std::decay_t<M>::ValueType)1 / rhs};
}







template<class Base> struct VectorWrapper;


template<class Base>
struct VectorWrapper : Base{

	template<SP_VECTOR_T(V)>
	const VectorWrapper &operator =(V &&rhs) noexcept{
		resize(*this, len(rhs));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] = rhs[i];
		return *this;
	}

	template<SP_VECTOR_T(V)>
	const VectorWrapper &operator +=(V &&rhs) noexcept{
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] += rhs[i];
		return *this;
	}
	template<SP_VECTOR_T(V)>
	const VectorWrapper &operator -=(V &&rhs) noexcept{
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] -= rhs[i];
		return *this;
	}
	
};


template<class M, class Cont, bool LV>
struct VectorExprPermute{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;
	const Cont *permuts;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool IsExpr = Arg::IsExpr;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	constexpr std::conditional_t<LV, ValueType &, ValueType>
		operator [](size_t i) noexcept{ return arg[(*permuts)[i]]; }
	constexpr ValueType operator [](size_t i) const noexcept{
		return arg[(*permuts)[i]];
	}
};

template<class M, class Cont, bool LV>
SP_CI size_t len(const VectorExprPermute<M, Cont, LV> &v) noexcept{
	return len(v.arg);
}

template<class M, class Cont, bool LV>
SP_CI size_t cap(const VectorExprPermute<M, Cont, LV> &v) noexcept{
	return cap(v.arg);
}

template<class M, class Cont, bool LV>
void resize(const VectorExprPermute<M, Cont, LV> &v, size_t size) noexcept{
	resize(v.arg, size);
}



template<class M, auto operation>
struct VectorExprElStatUnaryOp{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;


	constexpr ValueType operator [](size_t i) const noexcept{
		return operation(arg[i]);
	}
};

template<class M, auto Operation>
SP_CI size_t len(const VectorExprElStatUnaryOp<M, Operation> &v) noexcept{
	return len(v.arg);
}



template<class M, class Operation>
struct VectorExprElDynUnaryOp{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;
	Operation operation;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	constexpr ValueType operator [](size_t i) const noexcept{
		return operation(arg[i]);
	}
};

template<class M, class Operation>
SP_CI size_t len(const VectorExprElDynUnaryOp<M, Operation> &v) noexcept{
	return len(v.arg);
}



template<class T>
struct VectorExprCopy{
	static constexpr StupidVectorFlagType VectorFlag{};
	uint32_t data_index;
	uint32_t size;

	template<SP_VECTOR_T(V)>
	VectorExprCopy(V &&A) noexcept : data_index(len(MatrixTempStorage.data)), size(len(A)){
		expand_back(MatrixTempStorage.data, (len(*this) * sizeof(T) + 7) / 8);
		T *I = (T *)beg(MatrixTempStorage.data) + (size_t)data_index;
		for (size_t i=0; i!=size; ++i, ++I) *I = A[(size_t)i];
	}


	typedef T ValueType;
	constexpr static bool UsesBuffer = true;

	constexpr size_t capacity() const noexcept{ return 0; }

	constexpr ValueType operator [](size_t i) const noexcept{
		return *((T *)beg(MatrixTempStorage.data)+data_index + i);
	}
};

template<class T>
constexpr size_t len(const VectorExprCopy<T> &v) noexcept{ return v.size; }



template<class VL, class VR>
struct VectorExprAdd{
	static constexpr StupidVectorFlagType VectorFlag{};
	VL lhs;
	VR rhs;

	typedef std::remove_reference_t<VL> Lhs;
	typedef std::remove_reference_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	constexpr ValueType operator [](size_t i) const noexcept{
		return lhs[i] + rhs[i];
	}
};

template<class VL, class VR>
SP_CI size_t len(const VectorExprAdd<VL, VR> &v) noexcept{
	return len(v.lhs);
}



template<class VL, class VR>
struct VectorExprSubtract{
	static constexpr StupidVectorFlagType VectorFlag{};
	VL lhs;
	VR rhs;

	typedef std::remove_reference_t<VL> Lhs;
	typedef std::remove_reference_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	constexpr size_t len() const noexcept{ return len(lhs); }
	constexpr size_t capacity() const noexcept{ return 0; }

	constexpr ValueType operator [](size_t i) const noexcept{
		return lhs[i] - rhs[i];
	}
};

template<class VL, class VR>
SP_CI size_t len(const VectorExprSubtract<VL, VR> &v) noexcept{
	return len(v.lhs);
}



template<class VL, class VR>
struct VectorExprElMul{
	static constexpr StupidVectorFlagType VectorFlag{};
	VL lhs;
	VR rhs;

	typedef std::remove_reference_t<VL> Lhs;
	typedef std::remove_reference_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	constexpr ValueType operator [](size_t i) const noexcept{
		return lhs[i] * rhs[i];
	}
};

template<class VL, class VR>
SP_CI size_t len(const VectorExprElMul<VL, VR> &v) noexcept{
	return len(v.lhs);
}



template<class VL, class VR>
struct VectorExprElDiv{
	static constexpr StupidVectorFlagType VectorFlag{};
	VL lhs;
	VR rhs;

	typedef std::remove_reference_t<VL> Lhs;
	typedef std::remove_reference_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	constexpr ValueType operator [](size_t i) const noexcept{
		return lhs[i] / rhs[i];
	}
};

template<class VL, class VR>
SP_CI size_t len(const VectorExprElDiv<VL, VR> &v) noexcept{
	return len(v.lhs);
}



template<class VL, class VR, auto Operation>
struct VectorExprElStatOp{
	static constexpr StupidVectorFlagType VectorFlag{};
	VL lhs;
	VR rhs;

	typedef std::remove_reference_t<VL> Lhs;
	typedef std::remove_reference_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	constexpr ValueType operator [](size_t i) const noexcept{
		return Operation(lhs[i], rhs[i]);
	}
};

template<class VL, class VR, auto Operation>
SP_CI size_t len(const VectorExprElStatOp<VL, VR, Operation> &v) noexcept{
	return len(v.lhs);
}



template<class VL, class VR, class Operation>
struct VectorExprElDynOp{
	static constexpr StupidVectorFlagType VectorFlag{};
	VL lhs;
	VR rhs;
	Operation operation;


	typedef std::remove_reference_t<VL> Lhs;
	typedef std::remove_reference_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static bool UsesBuffer = Lhs::UsesBuffer || Rhs::UsesBuffer;

	constexpr ValueType operator [](size_t i) const noexcept{
		return operation(lhs[i], rhs[i]);
	}
};

template<class VL, class VR, class Operation>
SP_CI size_t len(const VectorExprElDynOp<VL, VR, Operation> &v) noexcept{
	return len(v.lhs);
}



template<class V>
struct VectorExprScalarMultiply{
	static constexpr StupidVectorFlagType VectorFlag{};
	typedef std::remove_reference_t<V> Arg;
	typedef typename Arg::ValueType ValueType;

	V lhs;
	ValueType rhs;

	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	constexpr ValueType operator [](size_t i) noexcept{
		return lhs[i] * rhs;
	}
};

template<class V>
SP_CI size_t len(const VectorExprScalarMultiply<V> &v) noexcept{
	return len(v.lhs);
}



template<class O>
SP_CSI auto vector_generator_helper_impl_(O &&op){
	if constexpr (std::is_invocable_v<O, size_t>)
		return op(0);
	if constexpr (std::is_invocable_v<O>)
		return op();
	else
		static_assert("generating function has wrong parameter types");
}

template<auto Operation>
struct VectorExprStatGenerator{
	static constexpr StupidVectorFlagType VectorFlag{};
	uint32_t size;

	typedef decltype(vector_generator_helper_impl_(Operation)) ValueType;
	constexpr static bool UsesBuffer = false;

	template<class O = decltype(Operation)> constexpr
	std::enable_if_t<std::is_invocable_v<O, size_t>, ValueType>
	operator [](size_t i) const noexcept{ return Operation(i); }
	
	template<class O = decltype(Operation)> constexpr
	std::enable_if_t<std::is_invocable_v<O>, ValueType>
	operator [](size_t i) const noexcept{ return Operation(); }
};

template<auto Operation>
SP_CI size_t len(const VectorExprStatGenerator<Operation> &v) noexcept{
	return v.size;
}



template<class Operation>
struct VectorExprDynGenerator{
	static constexpr StupidVectorFlagType VectorFlag{};
	Operation operation;
	uint32_t size;

	typedef decltype(vector_generator_helper_impl_(operation)) ValueType;
	constexpr static bool UsesBuffer = false;

	

	template<class O = Operation> constexpr
	std::enable_if_t<std::is_invocable_v<O, size_t>, ValueType>
	operator [](size_t i) const noexcept{ return operation(i); }
	
	template<class O = Operation> constexpr
	std::enable_if_t<std::is_invocable_v<O>, ValueType>
	operator [](size_t i) const noexcept{ return operation(); }
};

template<class Operation>
SP_CI size_t len(const VectorExprDynGenerator<Operation> &v) noexcept{
	return v.size;
}



template<class T>
struct VectorExprUniformValue{
	static constexpr StupidVectorFlagType VectorFlag{};
	T value;
	uint32_t size;

	typedef T ValueType;
	constexpr static bool UsesBuffer = false;

	constexpr ValueType operator [](size_t i) const noexcept{ return value; }
};

template<class T>
SP_CI size_t len(const VectorExprUniformValue<T> &v) noexcept{
	return v.size;
}








template<SP_VECTOR_T(V), class Cont>
auto perm(V &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(len(arg) != len(permuts),
		"permutation array must have the same length as permuted vector"
	);
	return VectorExprPermute<CRemRRef<V>, Cont, false>{arg, &permuts};
}
template<SP_VECTOR_T(V), class Cont>
auto l_perm(V &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(len(arg) != len(permuts),
		"permutation array must have the same length as permuted vector"
	);
	return VectorExprPermute<RemRRef<V>, Cont, true>{arg, &permuts};
}

template<SP_VECTOR_T(V)>
auto cp(V &&arg) noexcept{
	return VectorExprCopy<typename std::decay_t<V>::ValueType>{arg};
}

template<auto operation, SP_VECTOR_T(V)>
auto apply(V &&arg) noexcept{
	return VectorExprElStatUnaryOp<CRemRRef<V>, operation>{arg};
}

template<SP_VECTOR_T(V), class Operation>
auto apply(V &&arg, Operation &&operation) noexcept{
	return VectorExprElDynUnaryOp<CRemRRef<V>,
			decltype((Operation &&)operation)>{
				arg, (Operation &&)operation
			};
}

template<SP_VECTOR_T(VL), SP_VECTOR_T(VR)>
auto elwise_mul(VL &&lhs, VR &&rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs) != len(rhs),
		"element wise multiplied vectors cannot have different dimensions"
	);
	return VectorExprElMul<CRemRRef<VL>, CRemRRef<VR>>{lhs, rhs};
}

template<SP_VECTOR_T(VL), SP_VECTOR_T(VR)>
auto elwise_div(VL &&lhs, VR &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise divided vectors cannot have different dimensions"
	);
	return VectorExprElDiv<CRemRRef<VL>, CRemRRef<VR>>{lhs, rhs};
}

template<SP_VECTOR_T(VL), SP_VECTOR_T(VR), class Operation>
auto apply(VL &&lhs, VR &&rhs, Operation &&operation) noexcept{
	SP_MATRIX_ERROR(len(lhs)!=len(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExprElDynOp<CRemRRef<VL>, CRemRRef<VR>,
			 decltype((Operation &&)operation)>{
				 lhs, rhs, (Operation &&)operation
			 };
}


template<auto operation, SP_VECTOR_T(VL), SP_VECTOR_T(VR)>
auto apply(VL &&lhs, VR &&rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs)!=len(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExprElStatOp<CRemRRef<VL>, CRemRRef<VR>, operation>{lhs, rhs};
}








template<auto Operation>
auto generate(uint32_t size) noexcept{
	return VectorExprStatGenerator<Operation>{size};
}
template<class Operation>
auto generate(uint32_t size, Operation &&operation) noexcept{
	return VectorExprDynGenerator<decltype((Operation &&)operation)>{
		(Operation &&)operation, size
	};
}
template<class T>
auto uniform(uint32_t size, T value) noexcept{
	return VectorExprUniformValue<T>{value, size};
}







template<SP_VECTOR_T(VL), SP_VECTOR_T(VR)>
auto operator +(VL &&lhs, VR &&rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs)!=len(rhs), "added vectors cannot have different dimensions");
	return VectorExprAdd<CRemRRef<VL>, CRemRRef<VR>>{lhs, rhs};
}

template<SP_VECTOR_T(VL), SP_VECTOR_T(VR)>
auto operator -(VL &&lhs, VR &&rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs)!=len(rhs), "subtracted vectors cannot have different dimensions");
	return VectorExprSubtract<CRemRRef<VL>, CRemRRef<VR>>{lhs, rhs};
}


template<SP_VECTOR_T(V)>
auto operator *(V &&lhs, const typename std::decay_t<V>::ValueType &rhs) noexcept{
	return VectorExprScalarMultiply<CRemRRef<V>>{lhs, rhs};
}

template<SP_VECTOR_T(V)>
auto operator /(V &&lhs, const typename std::decay_t<V>::ValueType &rhs) noexcept{
	return VectorExprScalarMultiply<CRemRRef<V>>{lhs, (typename std::decay_t<V>::ValueType)1 / rhs};
}

} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////
