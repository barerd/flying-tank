(defsystem "flying-tank"
  :description "Flying Tank prototype game"
  :version "0.1"
  :author "Başar Erdivanlı"
  :license "MIT"
  :depends-on (:sdl2 :sdl2-image)
  :components ((:file "packages")
               (:file "main")))
