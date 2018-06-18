<?php
namespace App;
use Illuminate\Database\Eloquent\Model;
class FreelanceTask extends Model
{
    protected $fillable = ['finished'];
    protected $guarded = ['id', 'idFreelancer', 'idTask']; 
}