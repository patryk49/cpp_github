#pragma once

#include "Utils.hpp"

#define SP_ALLOCATORS

template<class A> constexpr
bool contains(const A &al, Memblock blk) noexcept{ return al.contains(blk); }

template<class A>
Memblock alloc(A &al, size_t size) noexcept{ return al.alloc(size); }
template<class A>
Memblock alloc(const A &al, size_t size) noexcept{ return al.alloc(size); }

template<class A>
void free(A &al, Memblock blk) noexcept{ return al.free(blk); }
template<class A>
void free(const A &al, Memblock blk) noexcept{ return al.free(blk); }

template<class A>
Memblock realloc(A &al, Memblock blk, size_t size) noexcept{ return al.realloc(blk, size); }
template<class A>
Memblock realloc(const A &al, Memblock blk, size_t size) noexcept{ return al.realloc(blk, size); }

template<class A>
void free_all(A &al) noexcept{ return al.free_all(); }
template<class A>
void free_all(const A &al) noexcept{ return al.free_all(); }



template<class T, class A> static 
Span<T> make_range(A &al, size_t size) noexcept{
	static_assert(alignof(T) <= A::Alignment, "this alignment is not supported by allocator");

	Memblock blk;
	if constexpr (A::Alignment)
		blk = alloc(al, size*sizeof(T));
	else
		blk = alloc(al, size*sizeof(T), alignof(T));
	return Span<T>{(T *)blk.ptr, blk.size / sizeof(T)};
}


struct AllocatorVtable{
};

struct Allocator{
	void *allocator;
	
	Memblock (*alloc_ptr)(void *, size_t);
	void (*free_ptr)(void *, Memblock);
	Memblock (*realloc_ptr)(void *, Memblock, size_t);
	
	bool (*contains_ptr)(const void *, Memblock);
	void (*free_all_ptr)(void *);

	size_t alignment;
	
	// TYPE CONVERSIONS
	Allocator() noexcept = default;
	Allocator(const Allocator &) noexcept = default;
	Allocator(Allocator &&) noexcept = default;
	Allocator &operator =(const Allocator &) noexcept = default;

	template<class A>
	Allocator(const A &al) noexcept{
		this->allocator = (void *)&al;

		this->alloc_ptr = (Memblock (*)(void *, size_t))&A::alloc;
		this->free_ptr = (void (*)(void *, Memblock))&A::free;
		this->realloc_ptr = (Memblock (*)(void *, Memblock, size_t))&A::realloc;

		if constexpr (A::IsAware)
			this->contains_ptr = (bool (*)(const void *, Memblock))&A::contains;
		else
			this->contains_ptr = nullptr;
		
		if constexpr (A::HasMassFree)
			this->free_all_ptr = (void (*)(void *))&A::free_all;
		else
			this->free_all_ptr = nullptr;

		this->alignment = A::Alignment;
	}
	
	template<class A>
	Allocator &operator =(const A &al) noexcept{
		this->allocator = (void *)&al;

		this->alloc_ptr = (Memblock (*)(void *, size_t))&A::alloc;
		this->free_ptr = (void (*)(void *, Memblock))&A::free;
		this->realloc_ptr = (Memblock (*)(void *, Memblock, size_t))&A::realloc;

		if constexpr (A::IsAware)
			this->contains_ptr = (bool (*)(const void *, Memblock))&A::contains;
		else
			this->contains_ptr = nullptr;
		
		if constexpr (A::HasMassFree)
			this->free_all_ptr = (void (*)(void *))&A::free_all;
		else
			this->free_all_ptr = nullptr;

		this->alignment = A::Alignment;
		return *this;
	}

	// PROCEDURES
	constexpr bool contains(Memblock blk) const noexcept{ return this->contains_ptr(this, blk); }
	
	Memblock alloc(size_t size) noexcept{ return this->alloc_ptr(this, size); }
	
	void free(Memblock blk) noexcept{ this->free_ptr(this, blk); }
	
	Memblock realloc(Memblock blk, size_t size) noexcept{
		return this->realloc_ptr(this, blk, size);
	}

	void free_all() noexcept{ this->free_all_ptr(this); }
};





// NULL ALLOCATOR
struct NullAllocator{
	static constexpr size_t Alignment = 0;
	static constexpr bool IsAware = true;
	static constexpr bool HasMassFree = true;
	
	// PROCEDURES
	constexpr
	bool contains(Memblock blk) noexcept{ return blk.ptr == nullptr; }
 
	Memblock alloc(size_t) noexcept{ return Memblock{nullptr, 0}; }

	constexpr
	void free(Memblock) noexcept{}

	constexpr
	Memblock realloc(Memblock, size_t) noexcept{ return Memblock{nullptr, 0}; }
	
	constexpr
	void free_all() noexcept{}
};




// FALLBACK ALLOCATOR
template<class A0, class A1>
struct FallbackAllocator{
	typedef std::remove_pointer_t<A0> MainAllocator;
	typedef std::remove_pointer_t<A1> SpareAllocator;

	static constexpr size_t Alignment = gcd(MainAllocator::Alignment, SpareAllocator::Alignment);
	static constexpr bool IsAware = SpareAllocator::IsAware;
	static constexpr bool HasMassFree = MainAllocator::HasMassFree && SpareAllocator::HasMassFree;

	static_assert(
		MainAllocator::IsAware,
		"main allocator of fallback allocator must be aware of what memory belongs to it"
	);

	[[no_unique_address]] A0 main;
	[[no_unique_address]] A1 spare;

	// PROCEDURES
	constexpr bool contains(Memblock blk) const noexcept{
		return contains(deref(this->main), blk) || contains(deref(this->spare), blk);
	}
	
	Memblock alloc(size_t size) noexcept{
		Memblock blk;
		blk = alloc(deref(this->main), size);
		if (blk.ptr) return blk;
		return alloc(deref(this->spare), size);
	}
	
	void free(Memblock blk) noexcept{
		if (contains(deref(this->main), blk))
			free(deref(this->main), blk);
		else
			free(deref(this->spare), blk);
	}
	
	Memblock realloc(Memblock blk, size_t size) noexcept{
		if (contains(deref(this->main), blk)){
			Memblock newblk;
			newblk = realloc(deref(this->main), blk, size);
		
			if (newblk.ptr) return newblk;
			
			newblk = alloc(deref(this->spare), size);
			
			if (newblk.ptr){
				for (uint8_t *I=blk.ptr, *J=newblk.ptr; I!=blk.ptr+blk.size; ++I, ++J) *J=*I;
				free(deref(this->main), blk);
			}
			return newblk;
		}
		return realloc(deref(this->spare), blk, size);
	}
	
	void free_all() noexcept{
		free(deref(this->main));
		free(deref(this->spare));
	}
};






// MALLOC ALLOCATOR
template<
	void *(*M)(size_t) = (void *(*)(size_t))::malloc,
	void (*F)(void *) = nullptr,
	void *(*R)(void *, size_t) = nullptr
>
struct MallocAllocator{
	static constexpr auto Malloc = M;
	static constexpr auto Free = (
		M==(void *(*)(size_t))::malloc ? (void (*)(void *))::free : F
	);
	static constexpr auto Realloc = (
		M==(void *(*)(size_t))::malloc ? (void *(*)(void *, size_t))::realloc : R
	);

	static constexpr size_t Alignment = alignof(max_align_t);
	static constexpr bool IsAware = false;
	static constexpr bool HasMassFree = false;
	
	// PROCEDURES
	constexpr bool contains(Memblock) noexcept{ return true; }

	Memblock alloc(size_t size) const noexcept{
		return Memblock{(uint8_t *)MallocAllocator<M, F, R>::Malloc(size), size};
	}

	void free(Memblock blk) const noexcept{
		MallocAllocator<M, F, R>::Free(blk.ptr);
	}

	Memblock realloc(Memblock blk, size_t size) const noexcept{
		if constexpr (MallocAllocator<M, F, R>::Realloc){
			return Memblock{(uint8_t *)MallocAllocator<M, F, R>::Realloc(blk.ptr, size), size};
		} else{
			Memblock new_blk;
			new_blk.ptr = (uint8_t *)MallocAllocator<M, F, R>::Malloc(size);
			new_blk.size = size;
			if (new_blk.ptr){
				for (uint8_t *I=beg(blk), *J=beg(new_blk); I!=end(blk); ++I, ++J) *J = *I;
				MallocAllocator<M, F, R>::Free(blk.ptr);
			}
			return new_blk;
		}
	}
};





// BUMP ALLOCATOR
template<size_t N = 0, size_t A = alignof(max_align_t)>
struct BumpAllocator{
	static constexpr size_t Size = N;

	static constexpr size_t Alignment = A;
	static constexpr bool IsAware = true;
	static constexpr bool HasMassFree = true;
	
	static_assert(!((Alignment-1) & -Alignment), "alignment must be a power of 2");
	static_assert(N % Alignment == 0, "storage size must be a multiple of the alignment");

	uint8_t *back = beg(storage);
	alignas(N!=0 ? A : alignof(intptr_t)) std::conditional_t<N==0, Memblock, uint8_t [N]> storage;

	// PROCEDURES
	constexpr bool contains(Memblock blk) noexcept{
		return beg(this->storage) <= blk.ptr && blk.ptr < this->back;
	}

	Memblock alloc(size_t size) noexcept{
		uint8_t *new_pointer = alignptr(this->back+size, A);
		size_t allocSize = new_pointer - this->back;

		Memblock blk{this->back, allocSize};

		if (new_pointer > end(this->storage)) return Memblock{nullptr, allocSize};
		
		this->back = new_pointer;
		return blk;
	}

	void free(BumpAllocator<N, A> &al, Memblock blk) noexcept{
		if (blk.ptr+blk.size == this->back) this->back = blk.ptr;
	}

	Memblock realloc(Memblock blk, size_t size) noexcept{
		if (blk.ptr+blk.size == this->back){
			uint8_t *new_back = alignptr(blk.ptr+size, A);
			if (new_back <= end(this->storage)){
				this->back = new_back;
				return Memblock{blk.ptr, new_back-blk.ptr};
			}
		}
		return this->alloc(size);
	}

	void free_all() noexcept{ this->back = beg(this->storage); }
};

template<size_t A = alignof(max_align_t)> static
BumpAllocator<0, A> make_bump_allocator(Memblock memory) noexcept{
	return BumpAllocator<0, A>{memory.ptr, memory};
}




// STACK ALLOCATOR
template<size_t N, size_t A = alignof(max_align_t)>
struct StackAllocator{
	static constexpr size_t Size = N;

	static constexpr size_t Alignment = A;
	static constexpr bool IsAware = true;
	static constexpr bool HasMassFree = true;
	
	static_assert(!((Alignment-1) & -Alignment), "alignment must be a power of 2");
	static_assert(N % Alignment == 0, "storage size must be a multiple of the alignment");

	uint8_t *back = beg(storage);
	uint8_t *front = beg(storage);
	alignas(N!=0 ? A : alignof(intptr_t)) std::conditional_t<N==0, Memblock, uint8_t [N]> storage;

	// PROCEDURES
	constexpr bool contains(Memblock blk) noexcept{
		return this->front <= blk.ptr && blk.ptr < this->back;
	}

	Memblock alloc(size_t size) noexcept{
		uint8_t *new_pointer = alignptr(this->back+size, A);
		size_t alloc_size = new_pointer - this->back;

		Memblock blk{this->back, alloc_size};

		if (new_pointer > end(this->storage)){
			if (alloc_size < (size_t)(this->front-beg(this->storage)))
				this->front -= alloc_size;
			else
				blk.ptr = nullptr;
			return blk;
		}
		
		this->back = new_pointer;
		return blk;
	}

	void free(Memblock blk) noexcept{
		if (blk.ptr+blk.size == this->back)
			this->back = blk.ptr;
		else if (blk.ptr == this->front)
			this->front = blk.ptr + blk.size;
	}

	Memblock realloc(Memblock blk, size_t size) noexcept{
		if (blk.ptr+blk.size == this->back){
			uint8_t *new_back = alignptr(blk.ptr+size, A);
			if (new_back <= end(this->storage)){		
				this->back = new_back;
				return Memblock{blk.ptr, new_back-blk.ptr};
			}
		} // else if (blk.ptr == this->front) // maybe add this

		return this->alloc(size);
	}

	void free_all() noexcept{
		this->back = beg(this->storage);
		this->front = beg(this->storage);
	}
};


template<size_t A = alignof(max_align_t)> static constexpr
StackAllocator<0, A> make_stack_allocator(Memblock memory) noexcept{
	return StackAllocator<0, A>{memory.ptr, memory.ptr, memory};
}



// FREE LIST
template<class A>
struct FreeListAllocator{
	typedef std::remove_pointer_t<A> BaseType;
	
	struct Node{
		Node *next;
		size_t size;
	};
	
	static constexpr size_t Alignment = BaseType::Alignment;
	static constexpr bool IsAware = BaseType::IsAware;
	static constexpr bool HasMassFree = BaseType::HasMassFree;
	
	static_assert(
		alignof(uint8_t *) <= Alignment,
		"aglignment of underlying allocator must be"
		"compltible with the alignment of the pointer"
	);


	[[no_unique_address]] A allocator;
	Node *head = nullptr;

	// PROCEDURES
	constexpr bool contains(Memblock blk) noexcept{
		for (const typename FreeListAllocator<A>::Node *node=this->head; node; node=node->next)
			if (blk.ptr == (uint8_t *)node) return true;
		return contains(this->allocator, blk);
	}
	
	Memblock alloc(size_t size) noexcept{
		typename FreeListAllocator<A>::Node **nodePtr = &this->head;
		for (typename FreeListAllocator<A>::Node *node=this->head; node; node=node->next){
			if (size <= node->size){
				if (size <= node->size/2){
					auto end = (typename FreeListAllocator<A>::Node *)alignptr(
						(uint8_t *)node+size, std::remove_pointer_t<A>::Alignment
					);
					size = (uint8_t *)end - (uint8_t *)node;
					end->next = node->next;
					end->size = node->size - size;
					*nodePtr = end;
					return Memblock{(uint8_t *)node, size};
				} else{
					*nodePtr = node->next;
					return Memblock{(uint8_t *)node, node->size};
				}
			}
			nodePtr = &node->next;
		}
		return alloc(deref(this->allocator), size);
	}
	
	void free(Memblock blk) noexcept{
		typename FreeListAllocator<A>::Node *prevHead = this->head;
		this->head = (typename FreeListAllocator<A>::Node *)blk.ptr;
		((typename FreeListAllocator<A>::Node *)blk.ptr)->next = prevHead;
		((typename FreeListAllocator<A>::Node *)blk.ptr)->size = blk.size;
	}
	
	Memblock realloc(Memblock blk, size_t size) noexcept{
		Memblock new_blk = this->alloc(size);
		if (new_blk.ptr){
			for (uint8_t *I=beg(blk), *J=beg(new_blk); I!=end(blk); ++I, ++J) *J = *I;
			this->free(blk);
		}
		return new_blk;
	}
	
	void free_all() noexcept{
		free(deref(this->allocator));
		this->head = nullptr;
	}
};










template<class A, auto >
struct ChainAllocator{
	static constexpr size_t Alignment = std::remove_pointer_t<A>::Alignment;
	static constexpr bool IsAware = std::remove_pointer_t<A>::IsAware;
	static constexpr bool HasMassFree = std::remove_pointer_t<A>::HasMassFree;
	
	uint8_t *storage;


};
