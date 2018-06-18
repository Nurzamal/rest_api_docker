<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class EnrollmentController extends Controller
{
     public function create()
    {
        return view('signup');
    }
     public function store()
    {
        $this->validate(request(), [
            'name' => 'required',
            'email' => 'required|email',
            'password' => 'required',
        ]);
        $user = User::create(request(['name', 'email', 'password', 'role']));
    
        auth()->login($user);
        
        return redirect()->route('home');
    }
}