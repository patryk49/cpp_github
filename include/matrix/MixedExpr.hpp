#pragma once

#include "Operations.hpp"


namespace sp{



template<class V, bool CVec, bool LV>
struct MatrixExprAsMatrix{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V arg;

	typedef std::remove_reference_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool isExpr = true;
	constexpr static bool RowMajor = CVec;
	constexpr static bool UndefMajor = false;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	constexpr std::conditional_t<LV, ValueType &, ValueType> operator ()(
		size_t r, size_t c
	) noexcept{ return arg[CVec ? r : c]; }
	
	[[nodiscard]] constexpr ValueType operator ()(size_t r, size_t c) const noexcept{
		return arg[CVec ? r : c];
	}
};

template<class V, bool CVec, bool LV>
SP_CI size_t rows(const MatrixExprAsMatrix<V, CVec, LV> &v) noexcept{ return CVec ? len(v.arg) : 1; }

template<class V, bool CVec, bool LV>
SP_CI size_t cols(const MatrixExprAsMatrix<V, CVec, LV> &v) noexcept{ return CVec ? 1 : len(v.arg); }

template<class V, bool CVec, bool LV>
SP_CI size_t len(const MatrixExprAsMatrix<V, CVec, LV> &v) noexcept{ return len(v.arg); }

template<class V, bool CVec, bool LV>
SP_CI size_t cap(const MatrixExprAsMatrix<V, CVec, LV> &v) noexcept{ return cap(v.arg); }

template<class V, bool CVec, bool LV>
void resize(MatrixExprAsMatrix<V, CVec, LV> &v, size_t r, size_t c) noexcept{
	SP_MATRIX_ERROR(
		r!=1 || c!=1,
		"vector interpreted as matrix cannot be resized to non vector dimensions"
	);
	resize(v, r+c-1);
};



template<class V, bool LV>
struct MatrixExprAsDiagonalMatrix{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V arg;

	typedef std::remove_reference_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool isExpr = true;
	constexpr static bool RowMajor = true;
	constexpr static bool UndefMajor = true;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	constexpr std::enable_if_t<LV, ValueType &> operator ()(size_t r, size_t c) noexcept{
		return arg[r];
	}
	constexpr ValueType operator ()(size_t r, size_t c) const noexcept{
		return r==c ? arg[r] : (ValueType)0;
	}
};

template<class V, bool LV>
SP_CI size_t rows(const MatrixExprAsDiagonalMatrix<V, LV> &v) noexcept{ return len(v.arg); }

template<class V, bool LV>
SP_CI size_t cols(const MatrixExprAsDiagonalMatrix<V, LV> &v) noexcept{ return len(v.arg); }

template<class V, bool LV>
SP_CI size_t len(const MatrixExprAsDiagonalMatrix<V, LV> &v) noexcept{ return len(v.arg)*len(v.arg); }

template<class V, bool LV>
SP_CI size_t cap(const MatrixExprAsDiagonalMatrix<V, LV> &v) noexcept{ return cap(v.arg); }

template<class V, bool LV>
void resize(MatrixExprAsDiagonalMatrix<V, LV> &v, size_t r, size_t c) noexcept{
	SP_MATRIX_ERROR(
		r != c,
		"vector interpreted as diagonal matrix cannot be resized to non square dimensions"
	);
	resize(v.arg, r);
};



template<class M, bool LV>
struct VectorExprAsRowVector{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;
	size_t rowIndex;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool isExpr = Arg::isExpr;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	constexpr std::conditional_t<LV, ValueType &, ValueType> operator [](size_t i) noexcept{
		return arg(rowIndex, i);
	}
	constexpr ValueType operator [](size_t i) const noexcept{ return arg(rowIndex, i); }
};

template<class M, bool LV>
SP_CI size_t len(const VectorExprAsRowVector<M, LV> &m) noexcept{ return cols(m.arg); }



template<class M, bool LV>
struct VectorExprAsColumnVector{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;
	size_t columnIndex;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static bool isExpr = Arg::isExpr;
	constexpr static bool UsesBuffer = Arg::UsesBuffer;

	constexpr std::conditional_t<LV, ValueType &, ValueType> operator [](size_t i) noexcept{ 
		return arg(i, columnIndex);
	}
	constexpr ValueType operator [](size_t i) const noexcept{ return arg(i, columnIndex); }
};

template<class M, bool LV>
SP_CI size_t len(const VectorExprAsColumnVector<M, LV> &m) noexcept{ return rows(m.arg); }



template<class M, class V>
struct VectorExprMatrixVertMultiply{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg1;
	V arg2;

	typedef std::remove_reference_t<M> Arg1;
	typedef std::remove_reference_t<V> Arg2;

	typedef typename Arg1::ValueType ValueType;
	constexpr static bool UsesBuffer = Arg1::UsesBuffer || Arg2::UsesBuffer;

	constexpr ValueType operator [](size_t i) const noexcept{
		ValueType res = (ValueType)0;
		for (size_t j=0; j!=len(arg2); ++j)
			res += arg1(i, j) * arg2[j];
		return res;
	}
};

template<class M, class V>
SP_CI size_t len(const VectorExprMatrixVertMultiply<M, V> &m) noexcept{ return rows(m.arg1); }



template<class V, class M>
struct VectorExprMatrixHoriMultiply{
	static constexpr StupidVectorFlagType VectorFlag{};
	V arg1;
	M arg2;

	typedef std::remove_reference_t<V> Arg1;
	typedef std::remove_reference_t<M> Arg2;

	typedef typename Arg1::ValueType ValueType;
	constexpr static bool UsesBuffer = Arg1::UsesBuffer || Arg2::UsesBuffer;

	constexpr ValueType operator [](size_t i) const noexcept{
		ValueType res = (ValueType)0;
		for (size_t j=0; j!=len(arg1); ++j)
			res += arg1[j] * arg2(j, i);
		return res;
	}
};

template<class V, class M>
SP_CI size_t len(const VectorExprMatrixHoriMultiply<V, M> &m) noexcept{ return cols(m.arg2); }



template<class V1, class V2>
struct MatrixExprOuterProd{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::ValueType ValueType;
	constexpr static bool RowMajor = false;
	constexpr static bool UndefMajor = true;
	constexpr static bool UsesBuffer = Arg1::UsesBuffer || Arg2::UsesBuffer;

	constexpr ValueType operator ()(size_t r, size_t c) const noexcept{
		return arg1[r] * arg2[c];
	}
};

template<class V1, class V2>
SP_CI size_t rows(const MatrixExprOuterProd<V1, V2> &m) noexcept{ return len(m.arg1); }

template<class V1, class V2>
SP_CI size_t cols(const MatrixExprOuterProd<V1, V2> &m) noexcept{ return len(m.arg2); }

template<class V1, class V2>
SP_CI size_t len(const MatrixExprOuterProd<V1, V2> &m) noexcept{ return len(m.arg1)*len(m.arg2); }



template<class V1, class V2, auto Operation>
struct MatrixExprOuterStatOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::ValueType ValueType;
	constexpr static bool RowMajor = false;
	constexpr static bool UndefMajor = true;
	constexpr static bool UsesBuffer = Arg1::UsesBuffer || Arg2::UsesBuffer;

	constexpr ValueType operator ()(size_t r, size_t c) const noexcept{ 
		return Operation(arg1[r], arg2[c]);
	}
};

template<class V1, class V2, auto Operation>
SP_CI size_t rows(const MatrixExprOuterStatOp<V1, V2, Operation> &m) noexcept{
	return len(m.arg1);
}

template<class V1, class V2, auto Operation>
SP_CI size_t cols(const MatrixExprOuterStatOp<V1, V2, Operation> &m) noexcept{
	return len(m.arg2);
}

template<class V1, class V2, auto Operation>
SP_CI size_t len(const MatrixExprOuterStatOp<V1, V2, Operation> &m) noexcept{
	return len(m.arg1)*len(m.arg2);
}



template<class V1, class V2, class Operation>
struct MatrixExprOuterDynOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V1 arg1;
	V2 arg2;
	Operation operation;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::ValueType ValueType;
	constexpr static bool RowMajor = false;
	constexpr static bool UndefMajor = true;
	constexpr static bool UsesBuffer = Arg1::UsesBuffer || Arg2::UsesBuffer;

	constexpr ValueType operator ()(size_t r, size_t c) const noexcept{
		return operation(arg1[r], arg2[c]);
	}
};

template<class V1, class V2, class Operation>
SP_CI size_t rows(const MatrixExprOuterDynOp<V1, V2, Operation> &m) noexcept{
	return len(m.arg1);
}

template<class V1, class V2, class Operation>
SP_CI size_t cols(const MatrixExprOuterDynOp<V1, V2, Operation> &m) noexcept{
	return len(m.arg2);
}

template<class V1, class V2, class Operation>
SP_CI size_t len(const MatrixExprOuterDynOp<V1, V2, Operation> &m) noexcept{
	return len(m.arg1)*len(m.arg2);
}



template<SP_VECTOR_T(V)>
[[nodiscard]] auto as_col(V &&arg) noexcept{
	return MatrixExprAsMatrix<CRemRRef<V>, true, false>{arg};
}
template<SP_VECTOR_T(V)>
auto l_as_col(V &&arg) noexcept{
	return MatrixWrapper<MatrixExprAsMatrix<RemRRef<V>, true, true>>{{arg}};
}

template<SP_VECTOR_T(V)>
[[nodiscard]] auto as_row(V &&arg) noexcept{
	return MatrixExprAsMatrix<CRemRRef<V>, false, false>{arg};
}
template<SP_VECTOR_T(V)>
auto l_as_row(V &&arg) noexcept{
	return MatrixWrapper<MatrixExprAsMatrix<RemRRef<V>, false, true>>{{arg}};
}

template<SP_VECTOR_T(V)>
[[nodiscard]] auto as_diagonal(V &&arg) noexcept{
	return MatrixExprAsDiagonalMatrix<CRemRRef<V>, false>{arg};
}
template<SP_VECTOR_T(V)>
auto l_as_diagonal(V &&arg) noexcept{
	return MatrixWrapper<MatrixExprAsDiagonalMatrix<RemRRef<V>, true>>{{arg}};
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto as_row(M &&arg, size_t rowIndex) noexcept{
	SP_MATRIX_ERROR(rowIndex >= rows(arg), "row index exceeds the scope matrix as vector reinterpretation");
	return VectorExprAsRowVector<CRemRRef<M>, false>{arg, rowIndex};
}
template<SP_MATRIX_T(M)>
auto l_as_row(M &&arg, size_t rowIndex) noexcept{
	SP_MATRIX_ERROR(rowIndex >= rows(arg), "row index exceeds the scope matrix as vector reinterpretation");
	return VectorWrapper<VectorExprAsRowVector<RemRRef<M>, true>>{{arg, rowIndex}};
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto as_col(M &&arg, size_t columnIndex) noexcept{
	SP_MATRIX_ERROR(columnIndex >= rows(arg), "column index exceeds the scope matrix as vector reinterpretation");
	return VectorExprAsColumnVector<CRemRRef<M>, false>{arg, columnIndex};
}
template<SP_MATRIX_T(M)>
auto l_as_col(M &&arg, size_t columnIndex) noexcept{
	SP_MATRIX_ERROR(columnIndex >= rows(arg), "column index exceeds the scope matrix as vector reinterpretation");
	return VectorWrapper<VectorExprAsColumnVector<RemRRef<M>, true>>{{arg, columnIndex}};
}


template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
[[nodiscard]] auto outer_prod(V1 &&lhs, V2 &&rhs) noexcept{
	return MatrixWrapper<MatrixExprOuterProd<CRemRRef<V1>, CRemRRef<V2>>>{{lhs, rhs}};
}

template<auto operation, SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
[[nodiscard]] auto outer_op(V1 &&lhs, V2 &&rhs) noexcept{
	return MatrixWrapper<MatrixExprOuterStatOp<CRemRRef<V1>, CRemRRef<V2>, operation>>{{lhs, rhs}};
}

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2), class Operation>
[[nodiscard]] auto outer_op(V1 &&lhs, V2 &&rhs, Operation &&operation) noexcept{
	return MatrixWrapper<MatrixExprOuterDynOp<CRemRRef<V1>, CRemRRef<V2>,
			decltype((Operation &&)operation)>>{{
				lhs, rhs, (Operation &&)operation
			}};
}



template<SP_MATRIX_T(M), SP_VECTOR_T(V)>
[[nodiscard]] auto operator *(M &&lhs, V &&rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != len(rhs),
		"vertically multipled vector must be of"
		" the same size as number of columns of multiplicating matrix"
	);
	return VectorWrapper<VectorExprMatrixVertMultiply<CRemRRef<M>, CRemRRef<V>>>{{lhs, rhs}};
}

template<SP_VECTOR_T(V), SP_MATRIX_T(M)>
[[nodiscard]] auto operator *(V &&lhs, M &&rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs) != rows(rhs),
		"horizontally multipled vector must be of"
		" the same size as number of columns of multiplicating matrix"
	);
	return VectorWrapper<VectorExprMatrixHoriMultiply<CRemRRef<V>, CRemRRef<M>>>{{lhs, rhs}};
}


} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////
