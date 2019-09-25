function swap(a,b)
    if not a or not b then
        error('Nil argument')
    end
    return b,a
end

function fail()
    swap('abc')
end

function hello()
    print('Hello!')
end

