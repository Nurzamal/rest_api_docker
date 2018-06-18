<?php
namespace App;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;
class Task extends Model
{
    use SoftDeletes;
    protected $fillable = ['title', 'description', 'price', 'deadline', 'active'];
    protected $guarded = ['id', 'idCustomer', 'idFreelancer']; 
    protected $dates = ['deleted_at'];
    public function users()
    {
        return $this->belongsToMany('App\User');
    }
}